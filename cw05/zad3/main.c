#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <wait.h>

int main(int argc, char **argv){

    pid_t pid_arr[6];

    if(mkfifo("pipe", 0777) < 0){
        printf("Nie mozna utworzyc potoku nazwanego\n");
        return -1;
    }
    for(int i=0; i<5; i++){
        char f_name[100];
        char num[2];
        strcpy(f_name, "pliki/plik");
        sprintf(num, "%d", i);
        strcat(f_name, num);
        FILE *f = fopen(f_name, "w+");
        for(int j=0;j<10; j++){
            char ch = '1' + i;
            fwrite(&ch, 1, 1, f);
        }
        fclose(f);
    }

    char* konsument [] = {"./konsument", "pipe", "./pliki/wyniki", "10", NULL};
    char* producent1 [] = {"./producent", "pipe", "./pliki/plik0", "5", NULL};
    char* producent2 [] = {"./producent", "pipe", "./pliki/plik1", "5", NULL};
    char* producent3 [] = {"./producent", "pipe", "./pliki/plik2", "5", NULL};
    char* producent4 [] = {"./producent", "pipe", "./pliki/plik3", "5", NULL};
    char* producent5 [] = {"./producent", "pipe", "./pliki/plik4", "5", NULL};


    pid_arr[0]=fork();
    if(pid_arr[0] == 0)
        execvp(producent1[0], producent1);
    
    pid_arr[1] = fork();
    if(pid_arr[1] == 0)
        execvp(producent2[0], producent2);

    pid_arr[2] = fork();
    if(pid_arr[2] == 0)
        execvp(producent3[0], producent3);

    pid_arr[3] = fork();
    if(pid_arr[3] == 0)
        execvp(producent4[0], producent4);

    pid_arr[4] = fork();
    if(pid_arr[4] == 0)
        execvp(producent5[0], producent5);

    pid_arr[5] = fork();
    if(pid_arr[5] == 0)
        execvp(konsument[0], konsument);

    for(int i=0; i<6; i++)
        waitpid(pid_arr[i], NULL, 0);

    return 0;
}