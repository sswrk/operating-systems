#ifndef WSPOLNY_H
#define WSPOLNY_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>
#include <time.h>

#define VALUE_LOW 1
#define VALUE_HIGH 100

#define ORDER_LIMIT 5

#define RECEIVERS 3
#define SENDERS 3
#define PACKERS 3
#define WORKERS (RECEIVERS + SENDERS + PACKERS)

#define ARRAY_IS_FREE 0
#define FIRST_FREE_INDEX 1
#define FIRST_PACK_INDEX 2
#define PACK_NUMBER 3
#define FIRST_SEND_INDEX 4
#define SEND_NUMBER 5
#define SEMAPHORES_NUMBER 6

typedef struct orders{
    int values[ORDER_LIMIT];
}orders;

typedef struct sembuf sembuf;

union semun{
    int value;
    struct semid_ds *buf;
    unsigned short *array;
    struct seminfo *__buf;
};

int get_semaphoreID();
int get_shmemoryID();

#endif //WSPOLNY_H