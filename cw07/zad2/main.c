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

pid_t pids_children[WORKERS];

void handle_sigint(int signum){
    for (int i = 0; i < WORKERS; i ++)
        kill(pids_children[i], SIGINT);
    for (int i = 0; i < SEMAPHORES_NUMBER; i ++){
        if (sem_unlink(NAMES_OF_SEMAPHORES[i]) < 0) { 
            printf("Nie udalo sie usunac semafora\n");  
            exit(-1); 
        }
    }
    if (shm_unlink(SHARED_MEMORY)) { 
        printf("Nie udalo sie usunac pamieci wspolnej\n");  
        exit(-1); 
    }
    exit(0);
}

int main(){

    signal(SIGINT, handle_sigint);
    
    //tworzenie semaforow
    sem_t *sem; 
    for(int i=0; i<WORKERS; i++){
        sem = sem_open(NAMES_OF_SEMAPHORES[i], O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO, 1);
        if (sem == SEM_FAILED) { printf("Nie udalo sie utworzyc semafora\n");  exit(-1); }
        sem_close(sem);
    }
    //tworzenie pamieci wspolnej
    int shared_memory = shm_open(SHARED_MEMORY, O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
    if (shared_memory < 0) { printf("Nie udalo sie utworzyc pamieci wspolnej\n");  exit(-1); }
    if (ftruncate(shared_memory, sizeof(orders)) < 0) { printf("Nie udalo sie ustawic rozmiaru pamieci\n");  exit(-1); }

    //uruchamianie pracownikow
    for (int i = 0; i < RECEIVERS; i ++){
        pid_t child_pid = fork();
        if (child_pid == 0){
            execlp("./odbierajacy", "odbierajacy", NULL);
        }
        pids_children[i] = child_pid;
    }
    for (int i = 0; i < PACKERS; i ++){
        pid_t child_pid = fork();
        if (child_pid == 0){
            execlp("./pakujacy", "pakujacy", NULL);
        }
        pids_children[i + RECEIVERS] = child_pid;
    }
    for (int i = 0; i < SENDERS; i ++){
        pid_t child_pid = fork();
        if (child_pid == 0){
            execlp("./wysylajacy", "wysylajacy", NULL);
        }
        pids_children[i + RECEIVERS + PACKERS] = child_pid;
    }
    for (int i = 0; i < WORKERS; i++){
        wait(NULL);
    }

    //usuwanie
    for (int i = 0; i < SEMAPHORES_NUMBER; i ++){
        if (sem_unlink(NAMES_OF_SEMAPHORES[i]) < 0) { 
            printf("Nie udalo sie usunac semafora\n");  
            exit(-1); 
        }
    }
    if (shm_unlink(SHARED_MEMORY)) { 
        printf("Nie udalo sie usunac pamieci wspolnej\n");  
        exit(-1); 
    }
    return 0;
}