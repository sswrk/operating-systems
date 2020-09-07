#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include "wspolny.h"

int s_queue;

int clients_a[CLIENTS_LIMIT];

key_t clients_q[CLIENTS_LIMIT];


void init(message *msg){
    int i = 0;
    while(i<CLIENTS_LIMIT && clients_q[i] != -1) i++;
    int new_id;
    if(i<CLIENTS_LIMIT) new_id = i+1;
    else return;

    message *message_reply =(message*)malloc(sizeof(message));
    message_reply->type = new_id;
    int client_q_id = msgget(msg->q_key, 0);
    if(client_q_id<0){
        printf("Nie mozna uzyskac dostapu do kolejki klienta!\n");
        exit(-1);
    }

    clients_a[new_id - 1] = 1;
    clients_q[new_id - 1] = msg->q_key;

    if(msgsnd(client_q_id, message_reply, SIZE_MESSAGE_LIMIT, 0) < 0 ){
        printf("Nie udalo sie wyslac komunikatu!\n");
        exit(-1);
    }
}

void quit(int signum){
    message *message_reply =(message*)malloc(sizeof(message));

    for(int i=0; i<CLIENTS_LIMIT; i++){
        key_t q_key = clients_q[i];
        if(q_key!=-1){
            message_reply->type = STOP;
            int client_q_id = msgget(q_key, 0);
            if(client_q_id < 0){
                printf("Nie mozna uzyskac dostepu do kolejki klienta!\n");
                exit(-1);
            }
            if(msgsnd(client_q_id, message_reply, SIZE_MESSAGE_LIMIT, 0)<0){
                printf("Nie udalo sie wyslac komunikatu!\n");
                exit(-1);
            }
            if(msgrcv(s_queue, message_reply, SIZE_MESSAGE_LIMIT, STOP, 0)<0){
                printf("Nie otrzymano komunikatu!\n");
                exit(-1);
            }
        }
    }
    msgctl(s_queue, IPC_RMID, NULL);
    exit(0);
}

void connect(message *msg){
    message *message_reply =(message*)malloc(sizeof(message));

    message_reply->type = CONNECT;
    message_reply->q_key = clients_q[msg->clientid_connected - 1];
    int client_q_id = msgget(clients_q[msg->clientid - 1], 0);
    if(client_q_id < 0){
        printf("Nie udalo sie uzyskac dostepu do kolejki klienta!\n");
        exit(-1);
    }
    if(msgsnd(client_q_id, message_reply, SIZE_MESSAGE_LIMIT, 0) < 0){
        printf("Nie udalo sie wyslac komunikatu!\n");
        exit(-1);
    }

    message_reply->type = CONNECT;
    message_reply->q_key = clients_q[msg->clientid - 1];
    message_reply->clientid = msg->clientid;
    client_q_id = msgget(clients_q[msg->clientid_connected - 1], 0);
    if(client_q_id < 0){
        printf("Nie udalo sie uzyskac dostepu do kolejki klienta!\n");
        exit(-1);
    }
    if(msgsnd(client_q_id, message_reply, SIZE_MESSAGE_LIMIT, 0) < 0){
        printf("Nie udalo sie wyslac komunikatu!\n");
        exit(-1);
    }

    clients_a[msg->clientid_connected - 1] = 0;
    clients_a[msg->clientid - 1] = 0;
}

void disconnect(message *msg){
    message *message_reply =(message*)malloc(sizeof(message));
    message_reply->type = DISCONNECT;

    int client_q_id = msgget(clients_q[msg->clientid_connected - 1], 0);
    if(client_q_id < 0){
        printf("Nie udalo sie uzyskac dostepu do kolejki klienta!\n");
        exit(-1);
    }
    if(msgsnd(client_q_id, message_reply, SIZE_MESSAGE_LIMIT, 0) < 0){
        printf("Nie udalo sie wyslac komunikatu!\n");
        exit(-1);
    }

    clients_a[msg->clientid_connected - 1] = 1;
    clients_a[msg->clientid - 1] = 1;
}

void list(message *msg){
    message *message_reply =(message*)malloc(sizeof(message));
    strcpy(message_reply->text, "");

    for(int i=0; i<CLIENTS_LIMIT; i++)
        if(clients_q[i] != -1)
            sprintf(message_reply->text + strlen(message_reply->text), 
                "ID: %d; klient jest dostepny: %d\n", i + 1, clients_a[i] == 1);

    int client_q_id = msgget(clients_q[msg->clientid - 1], 0);
    if(client_q_id < 0){
        printf("Nie udalo sie uzyskac dostepu do kolejki klienta!\n");
        exit(-1);
    }

    message_reply->type = msg->clientid;
    if(msgsnd(client_q_id, message_reply, SIZE_MESSAGE_LIMIT, 0) < 0){
        printf("Nie udalo sie wyslac komunikatu!\n");
        exit(-1);
    }
}

void stop(message *msg){
    clients_a[msg->clientid - 1] = 0;
    clients_q[msg->clientid - 1] = -1;
}

int main(){
    for(int i=0; i<CLIENTS_LIMIT; i++) clients_q[i] = -1;

    key_t q_key = ftok(getenv("HOME"), SERVER_KID);
    printf("Klucz serwera: %d\n", q_key);

    s_queue = msgget(q_key, IPC_CREAT);
    printf("Identyfikator kolejki komunikatow serwera: %d\n", s_queue);

    signal(SIGINT, quit);

    message *msg =(message*)malloc(sizeof(message));

    while(1){
        if(msgrcv(s_queue, msg, SIZE_MESSAGE_LIMIT, -1, 0) < 0){
            printf("Nie udalo sie odebrac komunikatu!\n");
            return -1;
        }
        switch(msg->type){
            case INIT:
                init(msg);
                break;
            case LIST:
                list(msg);
                break;
            case CONNECT:
                connect(msg);
                break;
            case DISCONNECT:
                disconnect(msg);
                break;
            case STOP:
                stop(msg);
                break;
            default:
                printf("Niepoprawny rodzaj komunikatu\n");
        }
    }
    return 0;
}