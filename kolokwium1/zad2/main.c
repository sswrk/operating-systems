#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>


void sighandler(int sig_no, siginfo_t *siginfo, void *context){
    printf("Child: %d\n", siginfo -> si_value.sival_int);
}

int main(int argc, char* argv[]) {

    if(argc != 3){
        printf("Not a suitable number of program parameters\n");
        return 1;
    }

    struct sigaction action;
    action.sa_sigaction = &sighandler;

    //..........
    action.sa_flags = SA_SIGINFO;
    sigset_t signal_set;
    sigfillset(&signal_set);
    sigdelset(&signal_set,SIGUSR1);


    int child = fork();
    if(child == 0) {
        //zablokuj wszystkie sygnaly za wyjatkiem SIGUSR1
        //zdefiniuj obsluge SIGUSR1 w taki sposob zeby proces potomny wydrukowal
        //na konsole przekazana przez rodzica wraz z sygnalem SIGUSR1 wartosc
        sigprocmask(SIG_BLOCK, &signal_set, NULL);
        sigemptyset(&action.sa_mask);
        sigaction(SIGUSR1, &action, NULL);
        pause();
    }
    else {
        //wyslij do procesu potomnego sygnal przekazany jako argv[2]
        //wraz z wartoscia przekazana jako argv[1]
        sleep(1);
        int send_val = atoi(argv[1]);
        __sigval_t val;
        val.sival_int = send_val;
        sigqueue(child, atoi(argv[2]), val);
    }

    return 0;
}