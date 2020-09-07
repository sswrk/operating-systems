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

int semaphoreID;
int shmemoryID;
pid_t pids_children[WORKERS];

void handler_sigint(int signum){
    for(int i = 0; i<WORKERS; i++){
        kill(pids_children[i], SIGINT);
    }
    semctl(semaphoreID, 0, IPC_RMID, NULL);
    shmctl(shmemoryID, IPC_RMID, NULL);
    exit(EXIT_SUCCESS);
}

int main(){
    signal(SIGINT, handler_sigint);
    
    //tworzenie semafora
    key_t semaphore_key = ftok(getenv("HOME"), 0);
    semaphoreID = semget(semaphore_key, SEMAPHORES_NUMBER, IPC_CREAT | 0666);
    if(semaphoreID<0){
        printf("Nie udalo sie stworzyc semafora\n");
        return -1;
    }
    union semun arg;
    arg.value = 0;
    semctl(semaphoreID, ARRAY_IS_FREE, SETVAL, arg);
    semctl(semaphoreID, FIRST_FREE_INDEX, SETVAL, arg);
    semctl(semaphoreID, FIRST_PACK_INDEX, SETVAL, arg);
    semctl(semaphoreID, PACK_NUMBER, SETVAL, arg);
    semctl(semaphoreID, FIRST_SEND_INDEX, SETVAL, arg);
    semctl(semaphoreID, SEND_NUMBER, SETVAL, arg);

    //tworzenie pamieci wspolnej
    key_t key = ftok(getenv("HOME"), 1);
    shmemoryID = shmget(key, sizeof(orders), IPC_CREAT | 0666);
    if(shmemoryID<0){
        printf("Nie udalo sie utworzyc pamieci wspolnej\n");
        return -1;
    }

    //uruchamianie pracownikow
    for(int i = 0; i<RECEIVERS; i++){
        pid_t child_pid = fork();
        if(child_pid==0){
            execlp("./odbierajacy", "odbierajacy", NULL);
        }
        pids_children[i] = child_pid;
    }
    for(int i = 0; i<PACKERS; i++){
        pid_t child_pid = fork();
        if(child_pid==0){
            execlp("./pakujacy", "pakujacy", NULL);
        }
        pids_children[i + RECEIVERS] = child_pid;
    }
    for(int i = 0; i<SENDERS; i++){
        pid_t child_pid = fork();
        if(child_pid==0){
            execlp("./wysylajacy", "wysylajacy", NULL);
        }
        pids_children[i + RECEIVERS + PACKERS] = child_pid;
    }
    for(int i = 0; i<WORKERS; i++){
        wait(NULL);
    }

    //usuwanie
    semctl(semaphoreID, 0, IPC_RMID, NULL);
    shmctl(shmemoryID, IPC_RMID, NULL);
    return 0;
}