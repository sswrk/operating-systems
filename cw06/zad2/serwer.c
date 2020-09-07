#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <mqueue.h>
#include "wspolny.h"

char* clients_q[CLIENTS_LIMIT];
int clients_a[CLIENTS_LIMIT];

mqd_t server_q_id;

void init(char *msg){
    int i = 0;
    while(i<CLIENTS_LIMIT && clients_q[i] != NULL) i++;
    int new_id;
    if(i<CLIENTS_LIMIT) new_id = i+1;
    else return;

    char *message_reply =(char*)calloc(MAX_MESSAGE_LEN ,sizeof(char));
    mqd_t client_q_id = mq_open(msg, O_RDWR, 0777, NULL);
    if(client_q_id<0){
        printf("Nie mozna uzyskac dostapu do kolejki klienta!\n");
        exit(-1);
    }

    clients_a[new_id - 1] = 1;
    clients_q[new_id - 1] =(char*)calloc(NAME_LEN, sizeof(char));
    strcpy(clients_q[new_id - 1], msg);

    if(mq_send(client_q_id, message_reply, MAX_MESSAGE_LEN, new_id) < 0 ){
        printf("Nie udalo sie wyslac komunikatu!\n");
        exit(-1);
    }
    if(mq_close(client_q_id) < 0 ){
        printf("Nie udalo sie zamknac kolejki klienta!\n");
        exit(-1);
    }
}

void quit(int signum){
    char *message_reply =(char*)calloc(MAX_MESSAGE_LEN, sizeof(char));

    for(int i=0; i <CLIENTS_LIMIT; i ++){
        if(clients_q[i] != NULL){
            mqd_t client_q_id = mq_open(clients_q[i], O_RDWR, 0777, NULL);
            if(client_q_id < 0){
                printf("Nie mozna uzyskac dostepu do kolejki klienta\n");
                exit(-1);
            }
            if(mq_send(client_q_id, message_reply, MAX_MESSAGE_LEN, STOP) < 0){
                printf("Nie udalo sie wyslac komunikatu!\n");
                exit(-1);
            }
            if(mq_receive(server_q_id, message_reply, MAX_MESSAGE_LEN, NULL) < 0){
                printf("Nie otrzymano komunikatu!\n");
                exit(-1);
            }
            if(mq_close(client_q_id) < 0){
                printf("Nie udalo sie zamknac kolejki!\n");
                exit(-1);
            }
        }
    }

    if(mq_close(server_q_id) < 0){
        printf("Nie udalo sie zamknac kolejki!\n");
        exit(-1);
    }
    if(mq_unlink(SERVER_QUEUE_NAME) < 0){
        printf("Nie mozna usunac kolejki serwera!\n");
        exit(-1);
    }
    exit(0);
}

void list(char *msg){
    char *message_reply =(char*)calloc(MAX_MESSAGE_LEN ,sizeof(char));
    int clientid =(int) msg[0];

    for(int i = 0; i < CLIENTS_LIMIT; i++)
        if(clients_q[i] != NULL)
            sprintf(message_reply + strlen(message_reply), "ID: %d; klient jest dostepny: %d\n", i + 1, clients_a[i] == 1);

    mqd_t client_q_id = mq_open(clients_q[clientid - 1], O_RDWR, 0777, NULL);
    if(client_q_id < 0){
        printf("Nie udalo sie uzyskac dostepu do kolejki klienta!\n");
        exit(-1);
    }

    if(mq_send(client_q_id, message_reply, MAX_MESSAGE_LEN, LIST) < 0){
        printf("Nie udalo sie wyslac komunikatu!\n");
        exit(-1);
    }
    if(mq_close(client_q_id) < 0 ){
        printf("Nie udalo sie zamknac kolejki klienta!\n");
        exit(-1);
    }
}

void connect(char *msg){
    char *message_reply =(char*)calloc(MAX_MESSAGE_LEN ,sizeof(char));
    int clientid =(int) msg[0];
    int other_client_id =(int) msg[1];

    mqd_t client_q_id = mq_open(clients_q[clientid - 1], O_RDWR, 0777, NULL);
    if(client_q_id < 0){
        printf("Nie udalo sie uzyskac dostepu do kolejki klienta!\n");
        exit(-1);
    }

    message_reply[0] = other_client_id;
    strcat(message_reply, clients_q[other_client_id - 1]);
    if(mq_send(client_q_id, message_reply, MAX_MESSAGE_LEN, CONNECT) < 0){
        printf("Nie udalo sie wyslac komunikatu!\n");
        exit(-1);
    }

    memset(message_reply, 0, strlen(message_reply));
    client_q_id = mq_open(clients_q[other_client_id - 1], O_RDWR, 0777, NULL);
    if(client_q_id < 0){
        printf("Nie udalo sie uzyskac dostepu do kolejki klienta!\n");
        exit(-1);
    }
    message_reply[0] = clientid;
    strcat(message_reply, clients_q[clientid - 1]);
    if(mq_send(client_q_id, message_reply, MAX_MESSAGE_LEN, CONNECT) < 0){
        printf("Nie udalo sie wyslac komunikatu!\n");
        exit(-1);
    }
    if(mq_close(client_q_id) < 0 ){
        printf("Nie udalo sie zamknac kolejki klienta!\n");
        exit(-1);
    }

    clients_a[other_client_id - 1] = 0;
    clients_a[clientid - 1] = 0;
}

void disconnect(char *msg){
    char *message_reply =(char*)calloc(MAX_MESSAGE_LEN ,sizeof(char));
    int clientid =(int) msg[0];
    int other_client_id =(int) msg[1];

    mqd_t client_q_id = mq_open(clients_q[other_client_id - 1], O_RDWR, 0777, NULL);
    if(client_q_id < 0){
        printf("Nie udalo sie uzyskac dostepu do kolejki klienta!\n");
        exit(-1);
    }
    if(mq_send(client_q_id, message_reply, MAX_MESSAGE_LEN, DISCONNECT) < 0){
        printf("Nie udalo sie wyslac komunikatu!\n");
        exit(-1);
    }
    if(mq_close(client_q_id) < 0 ){
        printf("Nie udalo sie zamknac kolejki klienta!\n");
        exit(-1);
    }

    clients_a[other_client_id - 1] = 1;
    clients_a[clientid - 1] = 1;
}

void stop(char *msg){
    printf("Stop\n");
    clients_a[(int) msg[0] - 1] = 0;
    clients_q[(int) msg[0] - 1] = NULL;
}

int main(){
    for(int i = 0; i < CLIENTS_LIMIT; i++) clients_q[i] = NULL;

    server_q_id = mq_open(SERVER_QUEUE_NAME, O_RDWR | O_CREAT, 0777, NULL);
    if(server_q_id < 0){
        printf("Nie udalo sie stworzyc kolejki!\n");
        return -1;
    }

    signal(SIGINT, quit);

    char*msg =(char*)calloc(MAX_MESSAGE_LEN, sizeof(char));
    unsigned int type;

    while(1){
        if(mq_receive(server_q_id, msg, MAX_MESSAGE_LEN, &type) < 0){
            printf("Nie udalo sie odebrac komunikatu!");
            return -1;
        }
        switch(type){
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