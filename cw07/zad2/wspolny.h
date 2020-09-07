#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>
#include <time.h>
#include <semaphore.h>

#define ORDER_LIMIT 5
#define VALUE_LOW 1
#define VALUE_HIGH 100

#define RECEIVERS 3
#define SENDERS 3
#define PACKERS 3

#define WORKERS (RECEIVERS + SENDERS + PACKERS)

#define SHARED_MEMORY "/SHARED_MEMORY"

#define SEMAPHORES_NUMBER 6

#define ARRAY_IS_FREE 0
#define FIRST_FREE_INDEX 1
#define FIRST_PACK_INDEX 2
#define PACK_NUMBER 3
#define FIRST_SEND_INDEX 4
#define SEND_NUMBER 5

const char* NAMES_OF_SEMAPHORES[6] = {"/ARRAY_IS_FREE", "/FIRST_FREE_INDEX", "/FIRST_PACK_INDEX", "/PACK_NUMBER", "/FIRST_SEND_INDEX", "/SEND_NUMBER"};

typedef struct orders{
    int values[ORDER_LIMIT];
}orders;


#endif //COMMON_H