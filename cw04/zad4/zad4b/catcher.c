#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <wait.h>

int CSIG;
int ENDSIG;
char* MODE;

int signals_c = 0;

union sigval value;

void handle_signals(int signal, siginfo_t *inf, void *uncontext){
    if(signal == CSIG){
        signals_c++;

        if(!strcmp("kill", MODE) || !strcmp("sigrt", MODE))
            kill(inf->si_pid, CSIG);
        else
            sigqueue(inf->si_pid, CSIG, value);
    }
    else if(signal == ENDSIG)
        if(!strcmp("kill", MODE) || !strcmp("sigrt", MODE)){
            kill(inf->si_pid, ENDSIG);
        else
            sigqueue(inf->si_pid, ENDSIG, value);
        printf("Catcher otrzymal %d sygnalow\n", signals_c);
        exit(0);
    }
}

int main(int argc, char **argv){
    if(argc != 2){
        printf("Niepoprawna liczba argumentow\n");
        return -1;
    }

    MODE = argv[1];

    if(!strcmp("queue", MODE)){
        CSIG = SIGUSR1;
        ENDSIG = SIGUSR2;
    }
    else if(!strcmp("kill", MODE)){
        CSIG = SIGUSR1;
        ENDSIG = SIGUSR2;
    }
    else if(!strcmp("sigrt", MODE)){
        CSIG = SIGRTMIN + 1;
        ENDSIG = SIGRTMIN + 2;
    }
    else{
        printf("Niepoprawny argument\n");
        return -1;
    }

    value.sival_int = 0;

    sigset_t mask;
    sigfillset(&mask);
    sigdelset(&mask, CSIG);
    sigdelset(&mask, ENDSIG);

    if(sigprocmask(SIG_BLOCK, &mask, NULL) < 0){
        printf("Nie mozna zablokowac sygnalow\n");
        return -1;
    }

    struct sigaction action;
    action.sa_flags = SA_SIGINFO;
    action.sa_sigaction = handle_signals;

    sigemptyset(&action.sa_mask);
    sigaddset(&action.sa_mask, CSIG);
    sigaddset(&action.sa_mask, ENDSIG);

    sigaction(CSIG, &action, NULL);
    sigaction(ENDSIG, &action, NULL);

    printf("Utworzono catcher z PID: %d\n", getpid());

    while(true) usleep(100);
    return 0;
}