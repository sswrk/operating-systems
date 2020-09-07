#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <signal.h>
#include "wspolny.h"
#include <time.h>

int clientid;
int q_id;
int server_q_id;
key_t q_key;


void init(){
    srand(time(NULL));

    q_key = ftok(getenv("HOME"), rand() % 255 + 1);
    printf("Klucz kolejki: %d\n", q_key);

    q_id = msgget(q_key, IPC_CREAT);
    if(q_id < 0){
        printf("Nie udalo sie stworzyc kolejki\n");
        exit(-1);
    }
    printf("Id kolejki: %d\n", q_id);

    key_t server_key = ftok(getenv("HOME"), SERVER_KID);
    server_q_id = msgget(server_key, 0);
    if(server_q_id < 0){
        printf("Nie udalo sie uzyskac dostepu do kolejki serwera\n");
        exit(-1);
    }
    printf("ID kolejki serwera: %d\n", server_q_id);

    message *msg =(message*)malloc(sizeof(message));
    msg->q_key = q_key;
    msg->type = INIT;
    if(msgsnd(server_q_id, msg, SIZE_MESSAGE_LIMIT, 0) < 0){
        printf("Nie udalo sie wyslac komunikatu\n");
        exit(-1);
    }

    message *message_reply =(message*)malloc(sizeof(message));
    if(msgrcv(q_id, message_reply, SIZE_MESSAGE_LIMIT, 0, 0) < 0){
        printf("Nie udalo sie otrzymac komunikatu\n");
        exit(-1);
    }
    clientid = message_reply->type;
    printf("ID klienta: %d\n", clientid);
}

void stop(){
    message *msg =(message*)malloc(sizeof(message));
    msg->clientid = clientid;
    msg->type = STOP;

    if(msgsnd(server_q_id, msg, SIZE_MESSAGE_LIMIT, 0) < 0){
        printf("Nie udalo sie wyslac komunikatu!\n");
        exit(-1);
    }
    if(msgctl(q_id, IPC_RMID, NULL) < 0){
        printf("Nie udalo sie usunac kolejki!\n");
        exit(-1);
    }
    msgctl(q_id, IPC_RMID, NULL);
    exit(0);
}


void msg_chat(int other_clientid, int other_q_id){
    message *msg =(message*)malloc(sizeof(message));
    char* cmd = NULL;
    size_t length = 0;
    ssize_t input = 0;

    while(1){
        printf("Wprowadz komunikat lub rozlacz sie: ");
        input = getline(&cmd, &length, stdin);
        cmd[input - 1] = '\0';

        if(msgrcv(q_id, msg, SIZE_MESSAGE_LIMIT, STOP, IPC_NOWAIT) >= 0){
            printf("Otrzymano STOP od serwera\n");
            stop();
        }

        if(msgrcv(q_id, msg, SIZE_MESSAGE_LIMIT, DISCONNECT, IPC_NOWAIT) >= 0)
            break;

        while(msgrcv(q_id, msg, SIZE_MESSAGE_LIMIT, 0, IPC_NOWAIT) >= 0)
            printf("Klient %d: %s\n", other_clientid, msg->text);

        if(strcmp(cmd, "DISCONNECT") == 0){
            msg->clientid = clientid;
            msg->clientid_connected = other_clientid;
            msg->type = DISCONNECT;
            if(msgsnd(server_q_id, msg, SIZE_MESSAGE_LIMIT, 0) < 0){
                printf("Nie udalo sie wyslac kominikatu!\n");
                exit(-1);
            }
            break;
        }
        else if(strcmp(cmd, "") != 0){
            msg->type = CONNECT;
            strcpy(msg->text, cmd);
            if(msgsnd(other_q_id, msg, SIZE_MESSAGE_LIMIT, 0) < 0){
                printf("Nie udalo sie wyslac komunikatu!\n");
                exit(-1);
            }
        }
    }
}

void list(){
    message *msg =(message*)malloc(sizeof(message));
    msg->clientid = clientid;
    msg->type = LIST;
    if(msgsnd(server_q_id, msg, SIZE_MESSAGE_LIMIT, 0) < 0){
        printf("Nie udalo sie wyslac komunikatu!\n");
        exit(-1);
    }

    message *message_reply =(message*)malloc(sizeof(message));
    if(msgrcv(q_id, message_reply, SIZE_MESSAGE_LIMIT, 0, 0) < 0){
        printf("Nie udalo sie odebrac komunikatu!\n");
        exit(-1);
    }

    printf("%s\n", message_reply->text);
}

void connect(int other_clientid){
    message *msg =(message*)malloc(sizeof(message));
    msg->type = CONNECT;
    msg->clientid = clientid;
    msg->clientid_connected = other_clientid;

    if(msgsnd(server_q_id, msg, SIZE_MESSAGE_LIMIT, 0) < 0){
        printf("Nie udalo sie wyslac komunikatu!\n");
        exit(-1);
    }

    message *message_reply =(message*)malloc(sizeof(message));
    if(msgrcv(q_id, message_reply, SIZE_MESSAGE_LIMIT, 0, 0) < 0){
        printf("Nie udalo sie odebrac komunikatu!\n");
        exit(-1);
    }
    key_t other_q_key = message_reply->q_key;
    int other_q_id = msgget(other_q_key, 0);
    if(other_q_id < 0){
        printf("Nie udalo sie odczytac klucza kolejki klienta\n");
        exit(-1);
    }

    msg_chat(other_clientid, other_q_id);
}

int main(){
    init();
    signal(SIGINT, stop);

    size_t length = 0;
    ssize_t input = 0;
    char* cmd = NULL;
    
    message *msg =(message*)malloc(sizeof(message));

    while(1){
        printf("Wprowadz komende: ");
        input = getline(&cmd, &length, stdin);
        cmd[input-1] = '\0';

        if(msgrcv(q_id, msg, SIZE_MESSAGE_LIMIT, 0, IPC_NOWAIT)>=0){
            if(msg->type == CONNECT){
                printf("Podlaczenie do klienta: %d\n", msg->clientid);
                int other_q_id = msgget(msg->q_key, 0);
                if(other_q_id < 1){
                    printf("Nie udalo sie uzyskac dostepu do kolejki drugiego klienta\n");
                    return -1;
                }
                msg_chat(msg->clientid, other_q_id);
            }
            else if(msg->type == STOP)
                stop();
        }

        if(strcmp(cmd, "") == 0) continue;

        char* buf = strtok(cmd, " ");
        if(!strcmp(buf, "STOP"))
            stop();
        else if(!strcmp(buf, "LIST"))
            list();
        else if(!strcmp(buf, "CONNECT")){
            buf = strtok(NULL, " ");
            int id = atoi(buf);
            connect(id);
        }
        else printf("Niepoprawna komenda\n");
    }
    return 0;
}