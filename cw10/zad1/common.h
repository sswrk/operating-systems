#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <poll.h>

#define CLIENT_LIMIT 20
#define MESSAGE_LIMIT 30
#define NAME_LENGTH 10

#define PING_INTERVAL 10
#define PING_WAIT 5

enum type_of_connection{
	LOCAL,
	INET
}; typedef enum type_of_connection type_of_connection;

enum type_of_message{
	CONNECT,
	CONNECT_FAILED,
	PING,
	WAIT,
	GAME_FOUND,
	MOVE,
	GAME_FINISHED,
	DISCONNECT,
	EMPTY
}; typedef enum type_of_message type_of_message;

struct game{
	char board[9];
	char turn;
	char winner;
}; typedef struct game game;

struct message{
    game game;
	type_of_message type_of_message;
	char name[NAME_LENGTH];
}; typedef struct message message;

struct client{
	int fd;
	char* name;	
	game* game;
	int is_active;
	int opponent_index;
	char symbol;
}; typedef struct client client;

void boardReset(game* game){
	for(int i = 0; i<9; i++) game->board[i] = '-';
	game->turn = 'O';
	game->winner = '-';
}

void messageSend(int fd, type_of_message type, game* game, char* name){
	char* message = calloc(MESSAGE_LIMIT, sizeof(char));
	if(type==CONNECT) sprintf(message, "%d %s",(int) type, name);
	else if(game==NULL) sprintf(message, "%d",(int) type);
	else sprintf(message, "%d %s %c %c",(int) type, game->board, game->turn, game->winner);

	if(write(fd, message, MESSAGE_LIMIT)<0){
        printf("Nie udalo sie wyslac komunikatu\n");
        exit(-1);
    }
	free(message);
}

message messageReceive(int fd, int nonblocking){

	message msg;
	int count;
	char* buffer = calloc(MESSAGE_LIMIT, sizeof(char));

	if(nonblocking &&(count = recv(fd, buffer, MESSAGE_LIMIT, MSG_DONTWAIT))<0){
		msg.type_of_message = EMPTY;
		free(buffer);
		return msg;
	}
	else if(!nonblocking &&(count = read(fd, buffer, MESSAGE_LIMIT))<0){
        printf("Nie udalo sie odebrac komunikatu\n");
        exit(-1);
    }
	if(count==0){
		msg.type_of_message = DISCONNECT;
		free(buffer);
		return msg;
	}

	char* token;
	char* rest = buffer;
	strcpy(msg.name, "");
	boardReset(&msg.game);
	token = strtok_r(rest, " ", &rest);

	if(nonblocking){
		char* p;
		msg.type_of_message =(type_of_message) strtol(token, &p, 10);
	}
	else msg.type_of_message =(type_of_message) atoi(token);

	if(msg.type_of_message==CONNECT){
		token = strtok_r(rest, " ", &rest);
		strcpy(msg.name, token);
	}
	else if(!nonblocking &&(msg.type_of_message==PING || msg.type_of_message==DISCONNECT || msg.type_of_message==WAIT)){
		free(buffer);
		return msg;
	}
	else if(msg.type_of_message==MOVE || msg.type_of_message==GAME_FINISHED || msg.type_of_message==GAME_FOUND){
		token = strtok_r(rest, " ", &rest);
		strcpy(msg.game.board, token);
		token = strtok_r(rest, " ", &rest);
		msg.game.turn = token[0];
		token = strtok_r(rest, " ", &rest);
		msg.game.winner = token[0];
	}

	free(buffer);
	return msg;
}

#endif //COMMON_H