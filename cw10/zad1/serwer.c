#include "common.h"

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
        printf("Nie udalo sie anulowac watku net\n");
        exit(-1);
    }
	if(pthread_cancel(ping_thread)==-1){
        printf("Nie udalo sie anulowac watku ping\n");
        exit(-1);
    }
	close(local_sock);
	unlink(socket_path);
	close(inet_sock);
}

void start(int port_number){
	local_sock = socket(AF_UNIX, SOCK_STREAM, 0);
	if(local_sock==-1){
        printf("Nie udalo sie zainicjalizowac lokalnego socketu\n");
        exit(-1);
    }

	local_sockaddr.sa_family = AF_UNIX;
	strcpy(local_sockaddr.sa_data, socket_path);

	if(bind(local_sock, &local_sockaddr, sizeof(local_sockaddr))<0){
        printf("Lokalny bind blad\n");
        exit(-1);
    }
	if(listen(local_sock, CLIENT_LIMIT)<0){
        printf("Lokalny listen blad\n");
        exit(-1);
    }

	printf("fd lokalnego socketu: %d\n", local_sock);

	inet_sock = socket(AF_INET, SOCK_STREAM, 0);
	if(inet_sock==-1){
        printf("Inicjalizacja socketu inet nie powiodla sie\n");
        exit(-1);
    }

	inet_sockaddr.sin_family = AF_INET;
	inet_sockaddr.sin_port = htons(port_number);
	inet_sockaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	if(bind(inet_sock,(struct sockaddr*) &inet_sockaddr, sizeof(inet_sockaddr))<0){
        printf("Inet bind blad\n");
        exit(-1);
    }
	if(listen(inet_sock, CLIENT_LIMIT)<0){
        printf("Inet listen blad\n");
        exit(-1);
    }
}

int clientExists(int i){
	return(i>-1 && i<CLIENT_LIMIT && clients[i].fd!=-1);
}

void clientDisconnect(int i){
	printf("Odlaczanie %s\n", clients[i].name);
	if(!(i>-1 && i<CLIENT_LIMIT && clients[i].fd!=-1)) return;
	if(shutdown(clients[i].fd, SHUT_RDWR)<0){
        printf("Nie udalo sie odlaczyc klienta\n");
        exit(-1);
    }
	if(close(clients[i].fd)<0){
        printf("Nie udalo sie zamknac klienta\n");
        exit(-1);
    }
}

void clientClear(int i){
	if(clients[i].name!=NULL) free(clients[i].name);
	clients[i].name = NULL;
	clients[i].fd = -1;
	clients[i].game = NULL;
	clients[i].is_active = 0;
	clients[i].symbol = '-';
	clients[i].opponent_index = -1;
	if(waiting_index==i) waiting_index = -1;
}

void gameStatusCheck(game* game){
	int wins[8][3] ={{0, 1, 2},{3, 4, 5},{6, 7, 8},{0, 3, 6},{1, 4, 7},{2, 5, 8},{0, 4, 8},{2, 4, 6} };

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
	if(!is_move) game->winner = 'D';

	if(game->turn=='X') game->turn = 'O';
	else if(game->turn=='O') game->turn = 'X';
}

int getFreeIndex(){
	for(int i = 0; i<CLIENT_LIMIT; i++){
		if(!(i>-1 && i<CLIENT_LIMIT && clients[i].fd!=-1)) return i;
	}
	return -1;
}

int nameIsFree(char* name){
	for(int i = 0; i<CLIENT_LIMIT; i++){
		if((i>-1 && i<CLIENT_LIMIT && clients[i].fd!=-1) && strcmp(name, clients[i].name)==0) return 0;
	}
	return 1;
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

	game* new_game = malloc(sizeof(game));
	boardReset(new_game);

	clients[id1].game = clients[id2].game = new_game;

	new_game->winner = clients[id1].symbol;
	messageSend(clients[id1].fd, GAME_FOUND, new_game, NULL);

	new_game->winner = clients[id2].symbol;
	messageSend(clients[id2].fd, GAME_FOUND, new_game, NULL);

	new_game->winner = '-';
}

void clientConnect(int fd){
	printf("Podlaczanie klienta\n");

	int client_fd = accept(fd, NULL, NULL);
	if(client_fd<0){
        printf("Nie zaakceptowano klienta\n");
        exit(-1);
    }

	message msg = messageReceive(client_fd, 0);

	char* name = calloc(NAME_LENGTH, sizeof(char));

	strcpy(name, msg.name);

	if(!nameIsFree(name)){
		messageSend(client_fd, CONNECT_FAILED, NULL, "Nazwa zajeta");
		return;
	}
	if(first_free==-1){
		messageSend(client_fd, CONNECT_FAILED, NULL, "Serwer jest zapelniony");
	}

	messageSend(client_fd, CONNECT, NULL, "Polaczono");


	clients[first_free].name = name;
	clients[first_free].is_active = 1;
	clients[first_free].fd = client_fd;


	for(int i = 0; i<CLIENT_LIMIT; i++)
		if(clients[i].name!=NULL) printf("%d: %s\n",i, clients[i].name);

	if(waiting_index!=-1){
		gameStart(first_free, waiting_index);
		waiting_index = -1;
	}
	else{
		waiting_index = first_free;
		messageSend(client_fd, WAIT, NULL, NULL);
		printf("WAIT wyslany\n");
	}

	first_free = getFreeIndex();

	printf("Polaczono\n");

}



void execNet(void* arg){

	struct pollfd poll_fds[CLIENT_LIMIT + 2];

	poll_fds[CLIENT_LIMIT].fd = local_sock;
	poll_fds[CLIENT_LIMIT + 1].fd = inet_sock;


	while(1){

		pthread_mutex_lock(&clients_mutex);

		for(int i = 0; i<CLIENT_LIMIT + 2; i++){
			if(i<CLIENT_LIMIT) poll_fds[i].fd = clients[i].fd;
			poll_fds[i].events = POLLIN;
			poll_fds[i].revents = 0;
		}

		pthread_mutex_unlock(&clients_mutex);

		if(poll(poll_fds, CLIENT_LIMIT + 2, -1)==-1){
            printf("Poll blad\n");
            exit(-1);
        }

		pthread_mutex_lock(&clients_mutex);

		for(int i = 0; i<CLIENT_LIMIT + 2; i++){
			if(i<CLIENT_LIMIT && !(i>-1 && i<CLIENT_LIMIT && clients[i].fd!=-1))
                continue;
			if(poll_fds[i].revents && POLLIN){
				if(poll_fds[i].fd==local_sock || poll_fds[i].fd==inet_sock)
					clientConnect(poll_fds[i].fd);
				else{
					message msg = messageReceive(poll_fds[i].fd, 0);
					if(msg.type_of_message==MOVE){
						printf("Otrzymano ruch\n");
						gameStatusCheck(&msg.game);
						if(msg.game.winner=='-')
							messageSend(clients[clients[i].opponent_index].fd, MOVE, &msg.game, NULL);
						else{
							messageSend(poll_fds[i].fd, GAME_FINISHED, &msg.game, NULL);
							messageSend(clients[clients[i].opponent_index].fd, GAME_FINISHED, &msg.game, NULL);
							free(clients[i].game);
						}
					}
					else if(msg.type_of_message==DISCONNECT){
						printf("Odlaczanie\n");
						if(clientExists(clients[i].opponent_index)){
							clientDisconnect(clients[i].opponent_index);
							clientClear(clients[i].opponent_index);
						}
						clientDisconnect(i);
						clientClear(i);
					}
					else if(msg.type_of_message==PING) clients[i].is_active = 1;
				}
			}
			else if((i>-1 && i<CLIENT_LIMIT && clients[i].fd!=-1) && poll_fds[i].revents && POLLHUP){
				printf("Odlaczanie\n");
				clientDisconnect(i);
				clientClear(i);
			}
		}
		pthread_mutex_unlock(&clients_mutex);
	}
}

void execPing(void* arg){
	while(1){
		sleep(PING_INTERVAL);
		printf("Start PING\n");
		pthread_mutex_lock(&clients_mutex);
		for(int i = 0; i<CLIENT_LIMIT; i++){
			if((i>-1 && i<CLIENT_LIMIT && clients[i].fd!=-1)){
				clients[i].is_active = 0;
				messageSend(clients[i].fd, PING, NULL, NULL);
			}
		}
		pthread_mutex_unlock(&clients_mutex);
		sleep(PING_WAIT);
		pthread_mutex_lock(&clients_mutex);
		for(int i = 0; i<CLIENT_LIMIT; i++){
			if((i>-1 && i<CLIENT_LIMIT && clients[i].fd!=-1) && clients[i].is_active==0){
				printf("Klient %s nie odpowiada. Odlaczanie %d\n", clients[i].name, i);
				messageSend(clients[i].fd, DISCONNECT, NULL, NULL);
				if(clientExists(clients[i].opponent_index)){
					clientDisconnect(clients[i].opponent_index);
					clientClear(clients[i].opponent_index);
				}
				clientDisconnect(i);
				clientClear(i);
			}
		}
		pthread_mutex_unlock(&clients_mutex);
		printf("Ping koniec\n");
	}

}
void handleSIGINT(int signo){
	exit(EXIT_SUCCESS);
}

int main(int argc, char** argv){

	if(argc<3){
        printf("Niepoprawna liczba agumentow\n");
        return -1;
    }
	signal(SIGINT, handleSIGINT);

	int port_number = atoi(argv[1]);
	socket_path = argv[2];

	atexit(serverClose);

	for(int i = 0; i<CLIENT_LIMIT; i++) clientClear(i);

	start(port_number);
	waiting_index = -1;
	first_free = 0;

	if(pthread_create(&net_thread, NULL,(void*) execNet, NULL)==-1){
        printf("Nie udalo sie stworzyc watku net\n");
        return -1;
    }
	if(pthread_create(&ping_thread, NULL,(void*) execPing, NULL)==-1){
        printf("Nie udalo sie stworzyc watku ping\n");
        return -1;
    }


	if(pthread_join(net_thread, NULL)<0){
        printf("Blad\n");
        return -1;
    }
	if(pthread_join(ping_thread, NULL)<0){
        printf("Blad\n");
        return -1;
    }

	serverClose();

	return 0;
}