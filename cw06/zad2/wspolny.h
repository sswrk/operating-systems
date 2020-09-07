#ifndef WSPOLNY_H
#define WSPOLNY_H

#define CLIENTS_LIMIT 10
#define MAX_MESSAGE_LEN 1000
#define NAME_LEN 20

typedef enum message_type{
    STOP = 1,
    DISCONNECT = 2,
    INIT = 3,
    LIST = 4,
    CONNECT = 5
}message_type;

const char* SERVER_QUEUE_NAME = "/SERVERQ";

#endif