#ifndef WSPOLNY_H
#define WSPOLNY_H

#define CLIENTS_LIMIT 10
#define MAX_MESSAGE_LEN 1000

typedef enum type{
    STOP = 1,
    DISCONNECT = 2,
    INIT = 3,
    LIST = 4,
    CONNECT = 5
}type;

typedef struct message{
    int clientid;
    int clientid_connected;
    long type;
    char text[MAX_MESSAGE_LEN];
    key_t q_key;
} message;

const int SIZE_MESSAGE_LIMIT = sizeof(message) - sizeof(long);
const int SERVER_KID = 5;

#endif