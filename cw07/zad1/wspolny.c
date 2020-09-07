#include "wspolny.h"

int get_semaphoreID(){
    key_t key = ftok(getenv("HOME"), 0);
    int id = semget(key, 0, 0);
    if(id<0){
        printf("Nie udalo sie otrzymac id semafora");
        exit(-1);
    }
    return id;
}

int get_shmemoryID(){
    key_t key = ftok(getenv("HOME"), 1);
    int id = shmget(key, 0, 0);
    if(id<0){
        printf("Nie mozna uzyskac dostepu do wspolnej pamieci");
        exit(-1);
    }
    return id;
}