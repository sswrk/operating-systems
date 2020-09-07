#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv){
    if(argc != 2){
        printf("Niepoprawna liczba argumentow\n");
        return -1;
    }

    if(!strcmp("mask", argv[1]) || !strcmp("pending", argv[1])){
        sigset_t m;
        sigpending(&m);
        if(sigismember(&m, SIGUSR1))
            printf("Sygnal oczekujacy w procesie-dziecku\n");
        else
            printf("Sygnal nie oczekujacy w procesie-dziecku\n");
    }
    else if(strcmp("pending", argv[1]) != 0)
        raise(SIGUSR1);

    return 0;
}