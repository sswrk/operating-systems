#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include "wspolny.h"

void order_pack(int semaphoreID, int shmemoryID){
    sembuf *sops =(sembuf *) calloc(4, sizeof(sembuf));

    sops[0].sem_num = ARRAY_IS_FREE;
    sops[0].sem_op = 0;

    sops[1].sem_num = ARRAY_IS_FREE;
    sops[1].sem_op = 1;

    sops[2].sem_num = FIRST_PACK_INDEX;
    sops[2].sem_op = 1;

    sops[3].sem_num = PACK_NUMBER;
    sops[3].sem_op = -1;

    for(int i = 0; i<4; i++)
        sops[i].sem_flg = 0;

    if(semop(semaphoreID, sops, 4)<0){
        printf("Nie udalo sie wykonac operacji na semaforach");
        exit(-1);
    }

    orders* orders = shmat(shmemoryID, NULL, 0);
    if(orders ==(void *) -1){
        printf("Nie mozna uzyskac dostepu do wspolnej pamieci");
        exit(-1);
    }

    int index = semctl(semaphoreID, FIRST_PACK_INDEX, GETVAL, NULL);
    if(index==-1){
        printf("Nie mozna pobrac indeksu");
        exit(-1);
    }
    index =(index - 1) % ORDER_LIMIT;

    orders->values[index] *= 2;

    int tosend_orders = semctl(semaphoreID, SEND_NUMBER, GETVAL, NULL) + 1;
    if(tosend_orders - 1==-1){
        printf("Nie mozna pobrac liczby zamowien");
        exit(-1);
    }
    
    int toprepare_orders = semctl(semaphoreID, PACK_NUMBER, GETVAL, NULL);
    if(toprepare_orders==-1){
        printf("Nie mozna pobrac liczby zamowien do przygotowania");
        exit(-1);
    }

    printf("[%d %ld] Pakujacy: Przygotowalem zamowienie o wielkosci: %d, Liczba zamowien do przygotowania: %d, Liczba zamowien do wyslania: %d.\n", 
            getpid(), time(NULL), orders->values[index], toprepare_orders, tosend_orders);

    if(shmdt(orders)==-1){
        printf("Nie udalo sie odlaczyc segmentu wspolnej pamieci");
        exit(-1);
    }

    
    sembuf *finalize = calloc(2, sizeof(sembuf));

    finalize[0].sem_num = ARRAY_IS_FREE;
    finalize[0].sem_op = -1;

    finalize[1].sem_num = SEND_NUMBER;
    finalize[1].sem_op = 1;

    for(int i = 0; i<2; i++)
        finalize[i].sem_flg = 0;
    
    if(semop(semaphoreID, finalize, 2)<0){
        printf("Nie udalo sie wykonac operacji na semaforach");
        exit(-1);
    }

}

int main(){
    srand(time(NULL));

    int semaphoreID = get_semaphoreID();
    int shmemoryID = get_shmemoryID();

    while(1){
        usleep((rand() %(1000-100+1)+100)*1000);
        if(semctl(semaphoreID, PACK_NUMBER, GETVAL, NULL)>0)
            order_pack(semaphoreID, shmemoryID);
    }
    return 0;
}