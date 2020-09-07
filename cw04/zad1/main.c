#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>

bool stop = false;

void signalSIGINT(){
    printf("\nOdebrano SIGINT\n");
    exit(0);
}

void signalSIGTSTP(){
    if(!stop)
        printf("\nCTRL+Z - kontynuuj CTRL+C - zakoncz\n");
    stop = !stop;
}

int main(){
    signal(SIGINT, signalSIGINT);
    struct sigaction action;
    action.sa_handler = signalSIGTSTP;
    action.sa_flags = 0;
    sigemptyset(&action.sa_mask);
    sigaction(SIGTSTP, &action, NULL);

    while(true){
        if(stop) pause();
        system("ls");
        sleep(2);
    }

    return 0;
}