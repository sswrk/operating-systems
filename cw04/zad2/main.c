#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <wait.h>

typedef enum Argument{
    PENDING,
    HANDLER,
    MASK,
    IGNORE
} Argument;

void handle(int signal){
    printf("Dostano sygnal\n");
}

int main(int argc, char **argv){
    if(argc != 3){
        printf("Niepoprawna liczba argumentow\n");
        return -1;
    }

    Argument argument;

    if(!strcmp("handler", argv[1])){
        argument = HANDLER;
        printf("handler\n");
        signal(SIGUSR1, handle);
    }
    else if(!strcmp("ignore", argv[1])){
        argument = IGNORE;
        printf("ignore\n");
        signal(SIGUSR1, SIG_IGN);
    }
    else if(!strcmp("mask", argv[1]) || !strcmp("pending", argv[1])){
        if(!strcmp("mask", argv[1])){
            argument = MASK;
            printf("mask\n");
        }
        else{
            argument = PENDING;
            printf("pending\n");
        }

        sigset_t mask;
        sigemptyset(&mask);
        sigaddset(&mask, SIGUSR1);

        if(sigprocmask(SIG_BLOCK, &mask, NULL) < 0){
            printf("Nie mozna zablokowac sygnalow\n");
            return -1;
        }

    } 
    else{
        printf("Niepoprawny argument\n");
        return -1;
    }

    sigset_t mask;

    raise(SIGUSR1);

    if(argument == MASK || argument == PENDING){
        sigpending(&mask);
        
        if(sigismember(&mask, SIGUSR1))
            printf("Sygnal oczekujacy w procesie-rodzicu\n");
    }

    if(!strcmp(argv[2], "fork")){
        pid_t pid_child = fork();

        if(pid_child == 0){
            if(argument != PENDING) raise(SIGUSR1);    
            if(argument == MASK || argument == PENDING){
                sigpending(&mask);   
                if(sigismember(&mask, SIGUSR1))
                    printf("Sygnal oczekujacy w procesie-dziecku\n");
                else
                    printf("Sygnal nie oczekujacy w procesie-dziecku\n");
            }
        }
    }
    else if(!strcmp(argv[2], "exec") && argument != HANDLER)
        execl("./exec",  "./exec", argv[1], NULL);
    
    wait(0);
    return 0;
}