#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/times.h>
#include <stdbool.h>

#define TIME_WORKER 5
#define TIME_WAIT 5
#define TIME_CLIENT_GENERATION 3

int clients_number;
int chairs_number;
int clients_waiting = 0;
int clients_done = 0;
bool worker_sleeping = false;
pthread_t client_inside;
pthread_t* chairs;
int chair_next_free = 0;
int chair_next_client = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void* worker(void* arg) {
    while(clients_done!=clients_number) {
        pthread_mutex_lock(&mutex);

        if(!clients_waiting){
            printf("Golibroda: ide spac\n");
            worker_sleeping = true;
            pthread_cond_wait(&cond, &mutex);
            worker_sleeping = false;
        }
        else{
            clients_waiting--;
            client_inside = chairs[chair_next_client];
            chair_next_client = (chair_next_client + 1) % chairs_number;
        }

        printf("Golibroda: czeka %d klientow, gole klienta %ld\n", clients_waiting, client_inside);
        pthread_cond_broadcast(&cond);
        pthread_mutex_unlock(&mutex);
        sleep(rand() % TIME_WORKER + 1);
        pthread_mutex_lock(&mutex);
        pthread_cancel(client_inside);
        clients_done++;
        client_inside = 0;

        pthread_mutex_unlock(&mutex);
    }
    pthread_exit((void *) 0);
}

void* client(void* arg) {
    pthread_t clientid = pthread_self();
    while(1) {
        pthread_mutex_lock(&mutex);

        if(worker_sleeping) {
            printf("Klient: budze golibrode; %ld\n", clientid);
            client_inside = clientid;
            pthread_cond_broadcast(&cond);
            pthread_mutex_unlock(&mutex);
            break;
        } 
        else if(clients_waiting < chairs_number) {
            chairs[chair_next_free] = clientid;
            chair_next_free = (chair_next_free + 1) % chairs_number;
            clients_waiting++;
            printf("Klient: poczekalnia, wolne miejsca %d; %ld\n", chairs_number - clients_waiting, clientid);
            pthread_mutex_unlock(&mutex);
            break;
        }
        printf("Klient: zajete; %ld\n", clientid);
        pthread_mutex_unlock(&mutex);
        sleep(rand() % TIME_WAIT + 1);
        
    }

    pthread_exit((void *) 0);
}


int main(int argc, char** argv) {
    if(argc != 3){
        printf("Nieprawidlowa liczba argumentow\n");
        return -1;
    }
    srand(time(NULL));

    chairs_number = atoi(argv[1]);
    clients_number = atoi(argv[2]);

    if (chairs_number<1 || clients_number<1){
        printf("Liczba krzesel i klientow musi byc dodatnia\n");
        return -1;
    }
    if(pthread_mutex_init(&mutex, NULL) != 0){
        printf("Mutex nie zostal zainicjalizowany\n");
        return -1;
    }

    chairs = (pthread_t*) calloc(chairs_number, sizeof(pthread_t));

    pthread_t worker_thread;
    pthread_create(&worker_thread, NULL, worker, NULL);

    pthread_t* client_threads = (pthread_t*) calloc(clients_number, sizeof(pthread_t));
    for(int i = 0; i < clients_number; i++) {
        sleep(rand() % TIME_CLIENT_GENERATION + 1);
        pthread_create(&client_threads[i], NULL, client, NULL);
    }

    for(int i = 0; i < clients_number; i++) {
        if(pthread_join(client_threads[i], NULL) > 0)
            return -1;
    }
    if (pthread_join(worker_thread, NULL) > 0)
        return -1;

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);

    return 0;
}