#include "common.h"

int port_number;
char* socket_path;

struct sockaddr local_sockaddr;
int local_sock;

struct sockaddr_in inet_sockaddr;
int inet_sock;

client clients[CLIENT_LIMIT];
int waiting_index;
int first_free;

pthread_mutex_t clients_mutex;
pthread_t net_thread;
pthread_t ping_thread;

void serverClose(){
	if(pthread_cancel(net_thread)==-1){
        printf("Nie mozna anulowac watku net\n");
        exit(-1);
    }
	if(pthread_cancel(ping_thread)==-1){
        printf("Nie mozna anulowac watku ping\n");
        exit(-1);
    }
	close(local_sock);
	unlink(socket_path);
	close(inet_sock);
}

void start(){
	local_sock = socket(AF_UNIX, SOCK_DGRAM, 0);
	if(local_sock==-1){
        printf("Nie udalo sie zainicjalizowac lokalnego socketu\n");
        exit(-1);
    }
	local_sockaddr.sa_family = AF_UNIX;
	strcpy(local_sockaddr.sa_data, socket_path);
	if(bind(local_sock, &local_sockaddr, sizeof(local_sockaddr))==-1){
        printf("Bind blad\n");
        exit(-1);
    }
	printf("fd lokalnego socketu\n: %d\n", local_sock);

	inet_sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(inet_sock==-1){
        printf("Nie udalo sie zainicjalizowac lokalnego socketu\n");
        exit(-1);
    }
	inet_sockaddr.sin_family = AF_INET;
	inet_sockaddr.sin_port = htons(port_number);
	inet_sockaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	if(bind(inet_sock,(struct sockaddr*) &inet_sockaddr, sizeof(inet_sockaddr))==-1){
        printf("Bind blad\n");
        exit(-1);
    }
}

void clientClear(int i){
	if(clients[i].name!=NULL) free(clients[i].name);
	if(clients[i].addr!=NULL) free(clients[i].addr);
	clients[i].name = NULL;
	clients[i].addr = NULL;
	clients[i].fd = -1;
	clients[i].game = NULL;
	clients[i].is_active = 0;
	clients[i].symbol = '-';
	clients[i].opponent_index = -1;
	if(waiting_index==i) waiting_index = -1;
}

int clientExists(int i){
	return i>= 0 && i<CLIENT_LIMIT && clients[i].fd!=-1;
}

int get_free_index(){
	for(int i = 0; i<CLIENT_LIMIT; i++){
		if(!clientExists(i)) return i;
	}
	return -1;
}

int get_client_index(char* name){
	for(int i = 0; i<CLIENT_LIMIT; i++){
		if(clientExists(i) && strcmp(name, clients[i].name)==0) return i;
	}
	return -1;
}

int is_name_available(char* name){
	for(int i = 0; i<CLIENT_LIMIT; i++){
		if(clientExists(i) && strcmp(name, clients[i].name)==0) return 0;
	}
	return 1;
}

void gameStatusCheck(game* game){
	static int wins[8][3] ={{0, 1, 2},{3, 4, 5},{6, 7, 8},{0, 3, 6},{1, 4, 7},{2, 5, 8},{0, 4, 8},{2, 4, 6} };

	for(int i = 0; i<8; i++){
		if(game->board[wins[i][0]]==game->board[wins[i][1]] && game->board[wins[i][1]]==game->board[wins[i][2]])
		{
			game->winner = game->board[wins[i][0]];
			return;
		}
	}

	int is_move = 0;
	for(int i = 0; i<9; i++){
		if(game->board[i]=='-'){
			is_move = 1;
			break;
		}
	}
	if(is_move==0) game->winner = 'D';
	if(game->turn=='X') game->turn = 'O';
	else if(game->turn=='O') game->turn = 'X';
}

void gameStart(int id1, int id2){

	clients[id1].opponent_index = id2;
	clients[id2].opponent_index = id1;

	if(rand() % 2==1){
		clients[id1].symbol = 'O';
		clients[id2].symbol = 'X';
	}
	else{
		clients[id1].symbol = 'X';
		clients[id2].symbol = 'O';
	}

	game* game = malloc(sizeof(game));
	boardReset(game);

	clients[id1].game = clients[id2].game = game;
	game->winner = clients[id1].symbol;
	messageSendTo(clients[id1].fd, clients[id1].addr, GAME_FOUND, game, clients[id1].name);
	game->winner = clients[id2].symbol;
	messageSendTo(clients[id2].fd, clients[id2].addr, GAME_FOUND, game, clients[id2].name);
	game->winner = '-';
}

void clientConnect(int fd, struct sockaddr* addr, char* new_nick){
	printf("Podlaczanie do serwera\n");
	char* nick = calloc(NAME_LENGTH, sizeof(char));
	strcpy(nick, new_nick);

	if(is_name_available(nick)==0){
		messageSendTo(fd, addr, CONNECT_FAILED, NULL, "Nazwa zajeta");
		free(nick);
		return;
	}
	if(first_free==-1){
		messageSendTo(fd, addr, CONNECT_FAILED, NULL, "Serwer jest pelny");
		free(nick);
		return;
	}
	messageSendTo(fd, addr, CONNECT, NULL, "Polaczono");

	clients[first_free].name = nick;
	clients[first_free].is_active = 1;
	clients[first_free].fd = fd;
	clients[first_free].addr = addr;

	for(int i = 0; i<CLIENT_LIMIT; i++)
		if(clients[i].name!=NULL) 
            printf("%d: %s\n",i, clients[i].name);

	if(waiting_index!=-1){
		gameStart(first_free, waiting_index);
		waiting_index = -1;
    }
	else{
		waiting_index = first_free;
		messageSendTo(fd, clients[first_free].addr, WAIT, NULL, nick);
    }
	first_free=get_free_index();

	printf("Klient polaczony\n");
}

void execNet(void* arg){
	struct pollfd poll_fds[2];

	poll_fds[0].fd = local_sock;
	poll_fds[1].fd = inet_sock;
	poll_fds[0].events = POLLIN;
	poll_fds[1].events = POLLIN;

	while(1){

		pthread_mutex_lock(&clients_mutex);
		for(int i = 0; i<2; i++){
			poll_fds[i].events = POLLIN;
			poll_fds[i].revents = 0;
		}
		pthread_mutex_unlock(&clients_mutex);
		if(poll(poll_fds, 2, -1)==-1){
            printf("Poll blad\n");
            exit(-1);
        }
		pthread_mutex_lock(&clients_mutex);

		for(int i = 0; i<2; i++){
			if(poll_fds[i].revents && POLLIN){

				struct sockaddr* addr = malloc(sizeof(struct sockaddr));
				socklen_t len = sizeof(&addr);
				printf("Deskryptor: %d\n", poll_fds[i].fd);

				message msg = messageReceiveFrom(poll_fds[i].fd, addr, len);
				printf("Komunikat odebrany na serwerze\n");
				int index;

				if(msg.type_of_message==CONNECT)
					clientConnect(poll_fds[i].fd, addr, msg.name);

				else if(msg.type_of_message==MOVE){
					printf("Odebrano ruch od gracza\n");
					index = get_client_index(msg.name);
					gameStatusCheck(&msg.game);

					if(msg.game.winner=='-')
						messageSendTo(clients[clients[index].opponent_index].fd, clients[clients[index].opponent_index].addr, MOVE, &msg.game, clients[clients[index].opponent_index].name);

					else{
						messageSendTo(clients[index].fd, clients[index].addr, GAME_FINISHED, &msg.game, clients[index].name);
						messageSendTo(clients[clients[index].opponent_index].fd, clients[clients[index].opponent_index].addr, GAME_FINISHED, &msg.game, clients[clients[index].opponent_index].name);
						free(clients[index].game);
					}
					free(addr);
				}
				else if(msg.type_of_message==DISCONNECT){
					index = get_client_index(msg.name);
					printf("Odlaczanie od serwera\n");

					if(clientExists(clients[index].opponent_index)){
						messageSendTo(clients[clients[index].opponent_index].fd, clients[clients[index].opponent_index].addr, DISCONNECT, NULL, clients[clients[index].opponent_index].name);
						clientClear(clients[index].opponent_index);
                    }

					clientClear(index);
					free(addr);
				}

				else if(msg.type_of_message==PING){
					index = get_client_index(msg.name);
					clients[index].is_active = 1;
					free(addr);
				}
				else free(addr);
			}
		}
		pthread_mutex_unlock(&clients_mutex);
	}
}

void execPing(void* arg){
	while(1){
		sleep(PING_INTERVAL);
		pthread_mutex_lock(&clients_mutex);
		for(int i = 0; i<CLIENT_LIMIT; i++){
			if(clientExists(i)){
				clients[i].is_active = 0;
				messageSendTo(clients[i].fd, clients[i].addr, PING, NULL, clients[i].name);
			}
		}
		pthread_mutex_unlock(&clients_mutex);
		sleep(PING_WAIT);
		pthread_mutex_lock(&clients_mutex);

		for(int i = 0; i<CLIENT_LIMIT; i++){

			if(clientExists(i) && clients[i].is_active==0){
				printf("Klient %d nie odpowiada. Odlaczanie %d\n", i, i);
				messageSendTo(clients[i].fd, clients[i].addr, DISCONNECT, NULL, clients[i].name);
				
				if(clientExists(clients[i].opponent_index)){
					messageSendTo(clients[clients[i].opponent_index].fd, clients[clients[i].opponent_index].addr, DISCONNECT, NULL, clients[clients[i].opponent_index].name);
					clientClear(clients[i].opponent_index);
				}
				clientClear(i);
			}
		}

		pthread_mutex_unlock(&clients_mutex);
	}
}


void exitFunction(){
	serverClose();
}

void handleSIGINT(int signo){
	exit(EXIT_SUCCESS);
}

int main(int argc, char** argv){

	if(argc<3){
        printf("Niepoprawna liczba argumentow\n");
        return -1;
    }
	srand(time(NULL));

	port_number = atoi(argv[1]);
	socket_path = argv[2];
	signal(SIGINT, handleSIGINT);
	atexit(exitFunction);

	for(int i = 0; i<CLIENT_LIMIT; i++) clientClear(i);

	start();
	waiting_index = -1;
	first_free = 0;

	if(pthread_create(&net_thread, NULL,(void*) execNet, NULL)==-1){
        printf("Nie mozna stworzyc nowego watku net\n");
        return -1;
    }
	if(pthread_create(&ping_thread, NULL,(void*) execPing, NULL)==-1){
        printf("Nie mozna stworzyc nowego watku ping\n");
        return -1;
    }

	if(pthread_join(net_thread, NULL)<0){
        printf("Blad");
        return -1;
    }
	if(pthread_join(ping_thread, NULL)<0){
        printf("Blad");
        return -1;
    }

	serverClose();
	return 0;
}