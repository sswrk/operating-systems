#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <wait.h>

void handle_child(int signal, siginfo_t *inf, void *uncontex){
    printf("Numer sygnalu: %d\n", inf->si_signo);
    printf("Kod wyjscia procesu-dziecka: %d\n", inf->si_status);
    printf("PID procesu wysylajacego: %d\n", inf->si_pid);
}

void handle_segfault(int signal, siginfo_t *inf, void *uncontex){
    printf("Numer sygnalu: %d\n", inf->si_signo);
    printf("PID procesu wysylajacego: %d\n", inf->si_pid);
    printf("Adres bledu: %p\n", inf->si_addr);
    exit(0);
}

void handle_status(int signal, siginfo_t *inf, void *uncontex){
    printf("Numer sygnalu: %d\n", inf->si_signo);
    printf("PID procesu wysylajacego: %d\n", inf ->si_pid);
    if(inf->si_code == SI_KERNEL)
        printf("Wyslany z kernel\n");
    else if(inf->si_code == SI_USER)
        printf("Wyslany przez uzytkownika\n");
}

int main(int argc, char **argv){
    if(argc != 2){
        printf("Niepoprawna liczba argumentow");
        return -1;
    }

    struct sigaction action;
    sigemptyset(&action.sa_mask);
    action.sa_flags = SA_SIGINFO;

    if(!strcmp("segfault", argv[1])){
        action.sa_sigaction = handle_segfault;
        sigaction(SIGSEGV, &action, NULL);
        char *chr = NULL;
        chr[10] = 'x';
    }
    else if(!strcmp("child", argv[1])){
        action.sa_sigaction = handle_child;
        sigaction(SIGCHLD, &action, NULL);
        pid_t pid_child = fork();
        if(pid_child == 0)
            return 1;
        wait(NULL);
    }
    else if(!strcmp("status", argv[1])){
        action.sa_sigaction = handle_status;
        sigaction(SIGINT, &action, NULL);
        pause();
    }
    else{
        printf("Niepoprawny argument");
        return -1;
    }

    return 0;
}