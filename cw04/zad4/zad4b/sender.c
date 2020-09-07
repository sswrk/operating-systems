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

int pid_catcher;
int signals_sendc;
int signals_c;

union sigval value;
int signals_sentc = 0;

void send_nextsig(){
    signals_sentc++;
    if(!strcmp("kill", MODE) || !strcmp("sigrt", MODE))
        kill(pid_catcher, CSIG);
    else
        sigqueue(pid_catcher, CSIG, value);
}

void send_endsig(){
     if(!strcmp("kill", MODE) || !strcmp("sigrt", MODE))
         kill(pid_catcher, ENDSIG);
     else
         sigqueue(pid_catcher, ENDSIG, value);
}

void handle_signals(int signal, siginfo_t *inf, void *uncontext){
    if(signal == CSIG){
        signals_c++;
        if(signals_sentc < signals_sendc)
            send_nextsig();
        else
            send_endsig();
    }
    else if(signal == ENDSIG){
        printf("Sender otrzyamal %d sygnalow; oczekiwana liczba sygnalow: %d\n", signals_c, signals_sendc);
        exit(0);
    }
}

int main(int argc, char **argv){

    if(argc != 4){
        printf("Niepoprawna liczba argumentow\n");
        return -1;
    }
    pid_catcher = atoi(argv[1]);
    signals_sendc = atoi(argv[2]);
    MODE = argv[3];

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

    printf("Utworzono sender z PID %d\n", getpid());

    send_nextsig();

    while(true) usleep(100);
    return 0;

}