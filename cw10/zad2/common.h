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
	type_of_message type_of_message;
	game game;
	char name[NAME_LENGTH];
}; typedef struct message message;

struct client{
	int fd;	
	struct sockaddr* addr;
	char* name;	
	int is_active;
	int opponent_index;
	game* game;
	char symbol;
}; typedef struct client client;

void boardReset(game* game){
	for(int i = 0; i<9; i++) game->board[i] = '-';
	game->turn = 'O';
	game->winner = '-';
}

void messageSend(int fd, type_of_message type, game* game, char* nick){
	char* message = calloc(MESSAGE_LIMIT, sizeof(char));
	if(game==NULL) sprintf(message, "%d %s",(int) type, nick);
	else sprintf(message, "%d %s %c %c %s",(int) type, game->board, game->turn, game->winner, nick);
	if(write(fd, message, MESSAGE_LIMIT)<0){
        printf("Nie udalo sie wyslac komunikatu\n");
        exit(-1);
    }
	free(message);
}

void messageSendTo(int fd, struct sockaddr* addr, type_of_message type, game* game, char* nick){
	char* message = calloc(MESSAGE_LIMIT, sizeof(char));
	if(game==NULL)sprintf(message, "%d %s",(int) type, nick);
	else sprintf(message, "%d %s %c %c %s",(int) type, game->board, game->turn, game->winner, nick);
	if(sendto(fd, message, MESSAGE_LIMIT, 0, addr, sizeof(struct sockaddr))<0){
        printf("Nie udalo sie wyslac komunikatu\n");
        exit(-1);
    }
	free(message);
}

message messageReceive(int fd){
	message msg;
	int count;
	char* msg_buf = calloc(MESSAGE_LIMIT, sizeof(char));

	if((count = read(fd, msg_buf, MESSAGE_LIMIT))<0){
        printf("Nie udalo sie odebrac komunikatu\n");
        exit(-1);
    }
	if(count==0){
		msg.type_of_message = DISCONNECT;
		free(msg_buf);
		return msg;
	}

	char* token;
	char* rest = msg_buf;
	strcpy(msg.name, "");
	boardReset(&msg.game);
	token = strtok_r(rest, " ", &rest);
	msg.type_of_message =(type_of_message) atoi(token);

	if(msg.type_of_message==CONNECT){
		token = strtok_r(rest, " ", &rest);
		strcpy(msg.name, token);
	}
	else if(msg.type_of_message==DISCONNECT || msg.type_of_message==PING || msg.type_of_message==WAIT){
		token = strtok_r(rest, " ", &rest);
		strcpy(msg.name, token);
		free(msg_buf);
		return msg;
	}
	else if(msg.type_of_message==MOVE || msg.type_of_message==GAME_FINISHED || msg.type_of_message==GAME_FOUND){
		token = strtok_r(rest, " ", &rest);
		strcpy(msg.game.board, token);
		token = strtok_r(rest, " ", &rest);
		msg.game.turn = token[0];
		token = strtok_r(rest, " ", &rest);
		msg.game.winner = token[0];
		token = strtok_r(rest, " ", &rest);
		strcpy(msg.name, token);
	}

	free(msg_buf);

	return msg;

}

message messageReceiveFrom(int fd, struct sockaddr* addr, socklen_t len){
	message msg;
	int count;
	char* msg_buf = calloc(MESSAGE_LIMIT, sizeof(char));

	if((count = recvfrom(fd, msg_buf, MESSAGE_LIMIT, 0, addr, &len))<0){
        printf("Nie udalo sie odebrac komunikatu\n");
        exit(-1);
    }

	if(count==0){
		msg.type_of_message=DISCONNECT;
		free(msg_buf);
		return msg;
	}

	char* token;
	char* rest = msg_buf;
	strcpy(msg.name, "");
	boardReset(&msg.game);
	token = strtok_r(rest, " ", &rest);
	msg.type_of_message =(type_of_message) atoi(token);

	switch(msg.type_of_message){
		case CONNECT:
			token = strtok_r(rest, " ", &rest);
			strcpy(msg.name, token);
			break;
		case PING: case WAIT: case DISCONNECT:
			token = strtok_r(rest, " ", &rest);
			strcpy(msg.name, token);
			free(msg_buf);
			return msg;
		case MOVE: case GAME_FOUND: case GAME_FINISHED:
			token = strtok_r(rest, " ", &rest);
			strcpy(msg.game.board, token);
			token = strtok_r(rest, " ", &rest);
			msg.game.turn = token[0];
			token = strtok_r(rest, " ", &rest);
			msg.game.winner = token[0];
			break;
		default:
			break;
	}

	free(msg_buf);

	return msg;

}

message messageReceiveNonblock(int fd){
	message msg;
	char* msg_buf = calloc(MESSAGE_LIMIT, sizeof(char));
	int count;

	if((count = recv(fd, msg_buf, MESSAGE_LIMIT, MSG_DONTWAIT))<0){
		msg.type_of_message = EMPTY;
		free(msg_buf);
		return msg;
	}
	
	if(count==0){
		msg.type_of_message = DISCONNECT;
		free(msg_buf);
		return msg;
	}

	char *token;
	char *rest = msg_buf;

	char* p;

	strcpy(msg.name, "");
	boardReset(&msg.game);
	token = strtok_r(rest, " ", &rest);

	msg.type_of_message =(type_of_message) strtol(token, &p, 10);

	switch(msg.type_of_message){
		case CONNECT:
			token = strtok_r(rest, " ", &rest);
			strcpy(msg.name, token);
			break;
		case PING: case WAIT: case DISCONNECT:
			token = strtok_r(rest, " ", &rest);
			strcpy(msg.name, token);
			free(msg_buf);
			return msg;
		case MOVE: case GAME_FOUND: case GAME_FINISHED:
			token = strtok_r(rest, " ", &rest);
			strcpy(msg.game.board, token);
			token = strtok_r(rest, " ", &rest);
			msg.game.turn = token[0];
			token = strtok_r(rest, " ", &rest);
			msg.game.winner = token[0];
			break;
		default:
			break;
	}

	free(msg_buf);

	return msg;

}

#endif //COMMON_H