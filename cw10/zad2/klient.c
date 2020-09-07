#include "common.h"

type_of_connection connection;
int port_number;
char* server;
char* name;
int server_fd;
char symbol;
int move;

void connectServerLocal(){
	struct sockaddr_un addr;
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, server);

	server_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
	if(server_fd<0){
        printf("Blad socket\n");
        exit(-1);
    }

	struct sockaddr_un c_addr;
	c_addr.sun_family = AF_UNIX;
	strcpy(c_addr.sun_path, name);

	if(bind(server_fd,(struct sockaddr*) &c_addr, sizeof(c_addr))<0){
        printf("Bind blad\n");
        exit(-1);
    }
	if(connect(server_fd,(struct sockaddr*) &addr, sizeof(addr))<0){
        printf("Nie udalo sie polaczyc z serwerem\n");
        exit(-1);
    }
}

void connectServerInet(){
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port_number);
	addr.sin_addr.s_addr = inet_addr(server);

	server_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if(server_fd<0){
        printf("Nie udalo sie stworzyc socketu\n");
        exit(-1);
    }

	struct sockaddr_in c_addr;
	c_addr.sin_family = AF_INET;
	c_addr.sin_port = 0;
	c_addr.sin_addr.s_addr = inet_addr(server);

	if(bind(server_fd,(struct sockaddr*) &c_addr, sizeof(c_addr))<0){
        printf("Nie mozna zbindowac\n");
        exit(-1);
    }
	if(connect(server_fd,(struct sockaddr*) &addr, sizeof(addr))<0){
        printf("Nie mozna polaczyc sie z serwerem\n");
        exit(-1);
    }
}

void serverDisconnect(){
	printf("Odlaczanie od serwera\n");
	messageSend(server_fd, DISCONNECT, NULL, name);
	if(connection==LOCAL) unlink(name);
}

void handleSIGINT(int signo){
	exit(EXIT_SUCCESS);
}

void exitFunction(){
	serverDisconnect();
}

void print_gameboard(game* game){
	printf("Twoj symbol: %c\n\n", symbol);

	for(int i = 0; i<9; i++){
		if(game->board[i]=='-') printf("%d", i);
		else printf("%c",game->board[i]);

		if(i % 3==2) printf("\n");
		else printf(" ");
	}

	printf("\n");
}

void moveEnemy(void* arg){
	message* msg =(message*) arg;
	printf("Twoj ruch: ");

	int move_char = getchar();
	move = move_char - '0';

	while(move<0 || move>8 || msg->game.board[move]!='-'){
		move_char = getchar();
		move = move_char - '0';
	}
	pthread_exit(0);
}

void moveMake(message *msg){
	move = -1;
	pthread_t move_thread;
	pthread_create(&move_thread, NULL,(void*) moveEnemy, msg);

	while(1){
		if(move<0 || move>8 || msg->game.board[move]!='-'){
			message rec_msg = messageReceiveNonblock(server_fd);
			if(rec_msg.type_of_message==DISCONNECT){
				handleSIGINT(SIGINT);
				exit(EXIT_SUCCESS);
			}
			else if(rec_msg.type_of_message==PING)
				messageSend(server_fd, PING, NULL, name);
		}
		else
			break;
	}

	pthread_join(move_thread, NULL);
	msg->game.board[move] = symbol;
	print_gameboard(&msg->game);
	messageSend(server_fd, MOVE, &msg->game, name);
}

void execClient(){
	while(1){
		message msg = messageReceive(server_fd);
		
		if(msg.type_of_message==WAIT) printf("Oczekiwanie na przeciwnika\n");
		else if(msg.type_of_message==PING) messageSend(server_fd, PING, NULL, name);
		else if(msg.type_of_message==DISCONNECT) exit(EXIT_SUCCESS);
		else if(msg.type_of_message==GAME_FOUND){
			symbol = msg.game.winner;
			printf("Twoj symbol: %c\n", symbol);
			print_gameboard(&msg.game);
			if(symbol=='O') moveMake(&msg);
			else printf("Oczekiwanie na przeciwnika\n");
		}
		else if(msg.type_of_message==MOVE){
			print_gameboard(&msg.game);
			moveMake(&msg);
		}
		else if(msg.type_of_message==GAME_FINISHED){
			print_gameboard(&msg.game);
			if(msg.game.winner==symbol) printf("Wygrales!\n");
			else if(msg.game.winner=='D') printf("Remis\n");
			else printf("Przegrales\n");
			exit(EXIT_SUCCESS);
		}
		else break;
	}
}

int main(int argc, char** argv){
	if(argc<4){
        printf("Niepoprawna liczba argumentow\n");
        return -1;
    }
	name = argv[1];
	atexit(exitFunction);
	signal(SIGINT, handleSIGINT);
	if(strcmp(argv[2], "LOCAL")==0) connection = LOCAL;
	else if(strcmp(argv[2], "INET")==0) connection = INET;
	else{
        printf("Niepoprawny typ polaczenia\n");
        return -1;
    }

	if(connection==LOCAL){
		server = argv[3];
		connectServerLocal();
	}
	else{
		if(argc<5){
            printf("Niepoprawne argumenty\n");
            return -1;
        }
		server = argv[3];
		port_number = atoi(argv[4]);
		printf("IP serwera: %s port: %d\n",server, port_number);
		connectServerInet();
	}

	messageSend(server_fd, CONNECT, NULL, name);
	message msg = messageReceive(server_fd);

	if(msg.type_of_message==CONNECT){
		printf("Klient jest podlaczony do serwera\n");
		execClient();
	}
	if(msg.type_of_message==CONNECT_FAILED){
		printf("Nie mozna polaczyc %s\n",msg.name);
		if(shutdown(server_fd, SHUT_RDWR)<0){
			printf("Nie mozna wylaczyc\n");
            return -1;
        }
		if(close(server_fd)<0){
			printf("Nie mozna zamknac deskryptora serwera\n");
            return -1;
        }
		exit(EXIT_FAILURE);
	}
	
	serverDisconnect();
}