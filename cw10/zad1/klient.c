#include "common.h"

char* playername;
int server_fd;
char symbol;
int move;
pthread_mutex_t mutex_move;

void serverDisconnect(){
	printf("Odlaczanie od serwera\n");
	messageSend(server_fd, DISCONNECT, NULL, NULL);
	if(shutdown(server_fd, SHUT_RDWR)<0){
        printf("Nie udalo sie wylaczyc\n");
        exit(-1);
    }
	if(close(server_fd)<0){
        printf("Nie udalo sie zamknac deskryptora serwera\n");
        exit(-1);
    }
	exit(EXIT_SUCCESS);
}

void connectServerLocal(char* server){
	struct sockaddr_un addr;
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, server);
	server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
	
	if(server_fd<0){ 
        printf("Blad socket\n");
        exit(-1);
    }
	if(connect(server_fd,(struct sockaddr*) &addr, sizeof(addr))<0){
        printf("Nie udalo sie polaczyc z serwerer\n");
        exit(-1);
    }
}

void connectServerInet(int port_number, char* server){
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port_number);
	addr.sin_addr.s_addr = inet_addr(server);
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	
	if(server_fd<0){
        printf("Blad socket\n");
        exit(-1);
    }
	if(connect(server_fd,(struct sockaddr*) &addr, sizeof(addr))<0){
        printf("Nie udalo sie polaczyc z serwerer\n");
        exit(-1);
    }
}


void boardDraw(game* game){
	printf("Twoj symbol: %c\n\n", symbol);
	for(int i = 0; i<9; i++){
		if(game->board[i]=='-') printf("%d", i);
		else printf("%c", game->board[i]);

		if(i%3==2) printf("\n");
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

void disconnect(){
	printf("Odlaczanie\n");
	serverDisconnect();
	exit(EXIT_SUCCESS);
}

void moveClient(message *msg){
	move = -1;
	pthread_t move_thread;
	pthread_create(&move_thread, NULL,(void*) moveEnemy, msg);

	while(1){
		if(move<0 || move>8 || msg->game.board[move]!='-'){
			message receive_msg = messageReceive(server_fd, 1);

			if(receive_msg.type_of_message==PING) messageSend(server_fd, PING, NULL, NULL);
			else if(receive_msg.type_of_message==DISCONNECT) disconnect();
		}
		else break;
	}

	pthread_join(move_thread, NULL);
	msg->game.board[move] = symbol;
	boardDraw(&msg->game);
	messageSend(server_fd, MOVE, &msg->game, NULL);
}


void gameEnd(game game){
	boardDraw(&game);
	if(game.winner==symbol) printf("Wygrales\n");
	else if(game.winner=='D') printf("Remis\n");
	else printf("Przegrales\n");
	serverDisconnect();
	exit(EXIT_SUCCESS);
}

void gameStart(message msg){
	symbol = msg.game.winner;
	printf("Twoj symbol: %c\n", symbol);
	boardDraw(&msg.game);
	if(symbol=='O') moveClient(&msg);
	else printf("Czekanie na przeciwnika\n");
}

void moveMake(message msg){
	printf("MOVE!\n");
	boardDraw(&msg.game);
	moveClient(&msg);
}

void execClient(){
	while(1){
		message msg = messageReceive(server_fd, 0);
		if(msg.type_of_message==WAIT) printf("Czekanie na przeciwnika\n");
		else if(msg.type_of_message==PING) messageSend(server_fd, PING, NULL, NULL);
		else if(msg.type_of_message==DISCONNECT) disconnect();
		else if(msg.type_of_message==GAME_FOUND) gameStart(msg);
		else if(msg.type_of_message==MOVE) moveMake(msg);
		else if(msg.type_of_message==GAME_FINISHED) gameEnd(msg.game);
		else break;
	}
}

void serverConnect(){
	messageSend(server_fd, CONNECT, NULL, playername);

	message msg = messageReceive(server_fd, 0);

	if(msg.type_of_message==CONNECT){
		printf("Polaczono sie z serwerem\n");
		execClient();
	}
	if(msg.type_of_message==CONNECT_FAILED){
		printf("Nie polaczyles sie z serwerem %s\n",msg.name);
		if(shutdown(server_fd, SHUT_RDWR)<0){
            printf("Nie udalo sie wylaczyc\n");
            exit(-1);
        }
		if(close(server_fd)<0){
            printf("Nie udalo sie zamknac deskryptora serwera\n");
            exit(-1);
        }
		exit(EXIT_FAILURE);
	}
	printf("Blad\n");
}


int main(int argc, char** argv){
	if(argc<4){
        printf("Niepoprawna licba argumentow\n");
        return -1;
    }
	srand(time(NULL));
	signal(SIGINT, serverDisconnect);

	playername = argv[1];

	type_of_connection connection;
	if(strcmp(argv[2], "LOCAL")==0) connection = LOCAL;
	else if(strcmp(argv[2], "INET")==0) connection = INET;
	else{
        printf("Niepoprawny typ polaczenia\n");
        return -1;
    }

	char* server = argv[3];
	if(connection==LOCAL)
		connectServerLocal(server);
	else{
		if(argc<5){
            printf("Niepoprawne argumenty\n");
            return -1;
        }
		printf("IP serwera: %s port: %d\n",server, atoi(argv[4]));
		connectServerInet(atoi(argv[4]), server);
	}

	serverConnect();
	serverDisconnect();
}