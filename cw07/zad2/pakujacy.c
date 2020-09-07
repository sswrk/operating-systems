#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>
#include <semaphore.h>

#include "wspolny.h"

sem_t* semaphores[SEMAPHORES_NUMBER];
int shmemory_desc;

void handle_sigint(int signum){
    for (int i = 0; i < SEMAPHORES_NUMBER; i ++){
        if (sem_close(semaphores[i]) < 0) {
            printf("Nie udalo sie zamknac semafora\n");  exit(-1);
        }
    }
    exit(EXIT_SUCCESS);
}

int getvalue(int index){
    int value;
    sem_getvalue(semaphores[index], &value);
    return value;
}

void order_pack(){
    sem_wait(semaphores[ARRAY_IS_FREE]);
    sem_post(semaphores[FIRST_PACK_INDEX]);
    sem_wait(semaphores[PACK_NUMBER]);

    orders *orders = mmap(NULL, sizeof(orders), PROT_READ | PROT_WRITE, MAP_SHARED, shmemory_desc, 0);
    if (orders == (void*) -1) { printf("Nie udalo sie dolaczyc pamieci\n");  exit(-1); }

    int index = (getvalue(FIRST_PACK_INDEX) - 1) % ORDER_LIMIT;
    orders -> values[index] *= 2;

    int tosend_orders = getvalue(SEND_NUMBER) + 1;
    int toprepare_orders = getvalue(PACK_NUMBER);

    printf("[%d %ld] Pakujacy: Przygotowalem zamowienie o wielkosci: %d, Liczba zamowien do przygotowania: %d, Liczba zamowien do wyslania: %d.\n", 
            getpid(), time(NULL), orders -> values[index], toprepare_orders, tosend_orders);

    if (munmap(orders, sizeof(orders)) < 0) { printf("Nie udalo sie odlaczyc pamieci\n");  exit(-1); }

    sem_post(semaphores[ARRAY_IS_FREE]);
    sem_post(semaphores[SEND_NUMBER]);

}

int main(){
    srand(time(NULL));

    signal(SIGINT, handle_sigint);

    //inicjalizacja semaforow
    for (int i = 0; i < SEMAPHORES_NUMBER; i ++){
        semaphores[i] = sem_open(NAMES_OF_SEMAPHORES[i], O_RDWR);
        if (semaphores[i] < 0) { printf("Nie udalo sie uzyskac dostepu do semafora\n");  exit(-1); }
    }
    
    //inicjalizacja wspolnej pamieci
    shmemory_desc = shm_open(SHARED_MEMORY, O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);
    if (shmemory_desc < 0) { printf("Nie udalo sie uzyskac dostepu do pamieci wspolnej\n");  exit(-1); }

    while(1){
        usleep((rand() %(1000-100+1)+100)*1000);
        if (getvalue(PACK_NUMBER) > 0)
            order_pack();
    }
    return 0;
}