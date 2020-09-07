#define LINE_LENGTH_LIMIT 1000
#define COMMAND_LIMIT 40
#define ARGUMENT_LIMIT 20
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

void cut(char **cmd_array, char*cmd){
    char* temp = cmd;
    char*argument = strtok_r(cmd, " ", &temp);
    for(int i=0; argument!=NULL && argument[0]!=EOF; i++){
        cmd_array[i] = argument;
        argument = strtok_r(NULL, " ", &temp);
    }
}

int main(int argc, char **argv){
    if(argc!=2){
        printf("Niepoprawna liczba argumentow\n");
        return -1;
    }
    FILE *f = fopen(argv[1], "r");
    if(f==NULL){
        printf("Nie mozna otworzyc pliku\n");
        return -1;
    }
    char *ln = malloc(LINE_LENGTH_LIMIT);
    while(fgets(ln, LINE_LENGTH_LIMIT,  f)!=NULL){
        if(ln[strlen(ln) - 1]=='\n')
            ln[strlen(ln) - 1] = '\0';
        printf("%s\n", ln);

        char* cmds [COMMAND_LIMIT][ARGUMENT_LIMIT];

        for(int i=0; i<COMMAND_LIMIT; i++)
            for(int j=0; j<ARGUMENT_LIMIT; j++)
                cmds[i][j] = NULL;

        char *temp = ln;
        char *cmd = strtok_r(ln, "|", &temp);
        int cmd_count = 0;
        
        while(cmd != NULL){
            cut(cmds[cmd_count], cmd);  
            cmd_count++;
            cmd = strtok_r(NULL, "|", &temp);
        }
        int pipes[COMMAND_LIMIT][2];
        for(int i=0; i<cmd_count-1; i++)
            if(pipe(pipes[i])<0){
                printf("Nie udalo sie utworzyc potoku\n");
                return -1;
            }
        for(int i=0; i<cmd_count; i++){
            pid_t pid = fork();

            if(pid < 0){
                printf("Nie mozna zforkowac\n");
                return -1;
            }
            else if(pid==0){
                if(i>0) dup2(pipes[i-1][0], STDIN_FILENO);
                if(i<cmd_count-1) dup2(pipes[i][1], STDOUT_FILENO);
                for(int j=0; j<cmd_count-1; j++){
                    close(pipes[j][0]);
                    close(pipes[j][1]);
                }
                execvp(cmds[i][0], cmds[i]);
                return 0;
            }
        }
        for(int j=0; j<cmd_count-1; j++){
            close(pipes[j][0]);
            close(pipes[j][1]);
        }
        for(int j=0; j<cmd_count; j++)
            wait(0);
    }
    free(ln);
    fclose(f);
    return 0;
}