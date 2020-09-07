#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <mqueue.h>
#include "wspolny.h"

char *q_name;
int clientid;
mqd_t q_id;
mqd_t server_q_id;

void init(){
    server_q_id = mq_open(SERVER_QUEUE_NAME, O_RDWR, 0777, NULL);
    if(server_q_id < 0){
        printf("Nie udalo sie uzyskac dostepu do kolejki serwera\n");
        exit(-1);
    }

    char* msg =(char*)calloc(MAX_MESSAGE_LEN, sizeof(char));
    strcpy(msg, q_name);
    if(mq_send(server_q_id, msg, MAX_MESSAGE_LEN, INIT) < 0){
        printf("Nie udalo sie otrzymac komunikatu!\n");
        exit(-1);
    }

    unsigned int id;
    if(mq_receive(q_id, msg, MAX_MESSAGE_LEN, &id) < 0){
        printf("Nie udalo sie otrzymac komunikatu!\n");
        exit(-1);
    }
    printf("ID klienta: %d\n", id);
    clientid = id;
}

void stop(){
    char *msg =(char*)calloc(MAX_MESSAGE_LEN, sizeof(char));
    msg[0] = clientid;

    if(mq_send(server_q_id, msg, MAX_MESSAGE_LEN, STOP) < 0){
        printf("Nie udalo sie wyslac komunikatu!\n");
        exit(-1);
    }
    if(mq_close(server_q_id)<0){
        printf("Nie udalo sie usunac kolejki!\n");
        exit(-1);
    }
    exit(0);
}


void msg_chat(int other_client_id, mqd_t other_queue){
    char *msg =(char*)calloc(MAX_MESSAGE_LEN, sizeof(char));
    char* cmd = NULL;
    size_t length = 0;
    ssize_t input = 0;

    while(1){
        printf("Wprowadz komunikat lub rozlacz sie: ");
        input = getline(&cmd, &length, stdin);
        cmd[input - 1] = '\0';

        struct timespec* timespec =(struct timespec*)malloc(sizeof(struct timespec));
        unsigned int type;
        int disconnected = 0;

        while(mq_timedreceive(q_id, msg, MAX_MESSAGE_LEN, &type, timespec) >= 0){
            if(type == STOP){
                printf("Otrzymano stop od serwera\n");
                stop();
            }
            else if(type == DISCONNECT){
                disconnected = 1;
                break;
            }
            else
                printf("Klient %d: %s\n", other_client_id, msg);
        }
        if(disconnected == 1) break;

        if(strcmp(cmd, "DISCONNECT") == 0){
            msg[0] = clientid;
            msg[1] = other_client_id;
            if(mq_send(server_q_id, msg, MAX_MESSAGE_LEN, DISCONNECT) < 0){
                printf("Nie udalo sie wyslac komunikatu!\n");
                exit(-1);
            }
            break;
        }
        else if(strcmp(cmd, "") != 0){
            strcpy(msg, cmd);
            if(mq_send(other_queue, msg, MAX_MESSAGE_LEN, CONNECT) < 0){
                printf("Nie udalo sie wyslac komunikatu!\n");
                exit(-1);
            }
        }
    }

}

void list(){
    char *msg =(char*)calloc(MAX_MESSAGE_LEN, sizeof(char));
    msg[0] = clientid;
    if(mq_send(server_q_id, msg, MAX_MESSAGE_LEN, LIST) < 0){
        printf("Nie udalo sie wyslac komunikatu!\n");
        exit(-1);
    }

    if(mq_receive(q_id, msg, MAX_MESSAGE_LEN, NULL) < 0){
        printf("Nie udalo sie odebrac komunikatu!\n");
        exit(-1);
    }

    printf("%s\n", msg);
}


void connect(int other_client_id){
    char *msg =(char*)calloc(MAX_MESSAGE_LEN, sizeof(char));
    msg[0] = clientid;
    msg[1] = other_client_id;

    if(mq_send(server_q_id, msg, MAX_MESSAGE_LEN, CONNECT) < 0){
        printf("Nie udalo sie wyslac komunikatu!\n");
        exit(-1);
    }
    if(mq_receive(q_id, msg, MAX_MESSAGE_LEN, NULL) < 0){
        printf("Nie dualo sie odebrac komunikatu!\n");
        exit(-1);
    }

    char *other_q_name =(char*)calloc(MAX_MESSAGE_LEN, sizeof(char));
    strncpy(other_q_name, msg + 1, strlen(msg) - 1);
    printf("Druga kolejka: %s\n", other_q_name);

    mqd_t other_queue = mq_open(other_q_name, O_RDWR, 0777, NULL);
    if(other_queue < 0){
        printf("Nie udalo sie odczytac klucza kolejki klienta\n");
        exit(-1);
    }

    msg_chat(other_client_id, other_queue);
}

int main(int argc, char**argv){
    q_name = argv[1];
    q_id = mq_open(q_name, O_RDWR | O_CREAT, 0777, NULL);
    if(q_id < 0){
        printf("Nie udalo sie stworzyc kolejki!\n");
        return -1;
    }
    init();
    signal(SIGINT, stop);

    char* cmd = NULL;
    size_t length = 0;
    ssize_t input = 0;

    char *msg =(char*)calloc(MAX_MESSAGE_LEN, sizeof(char));
    struct timespec* timespec =(struct timespec*)malloc(sizeof(struct timespec));
    unsigned int type;

    while(1){
        printf("Wprowadz komende: ");
        input = getline(&cmd, &length, stdin);
        cmd[input - 1] = '\0';


        
        if(mq_timedreceive(q_id, msg, MAX_MESSAGE_LEN, &type, timespec)>=0){
            if(type == CONNECT){
                printf("Podlaczanie do klienta\n");
                char* other_q_name =(char*)calloc(NAME_LEN, sizeof(char));
                strncpy(other_q_name, msg + 1, strlen(msg) - 1);
                printf("Druga kolejka: %s\n", other_q_name);
                mqd_t other_q_id = mq_open(other_q_name, O_RDWR, 0777, NULL);
                if(other_q_id < 0){
                    printf("Nie udalo sie uzyskac dostepu do kolejki drugiego klienta\n");
                    return -1;
                }
                msg_chat((int) msg[0], other_q_id);
            }
            else if(type == STOP)
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