#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#define __USE_XOPEN_EXTENDED 1
#include <ftw.h>
#include <stdbool.h>
#include <wait.h>
#include <unistd.h>

static int find(const char *path, const struct stat *sb, int typeflag, struct FTW *ftwbuf){
    if (S_ISDIR(sb -> st_mode)){
        pid_t pid_fork = fork();
        if (pid_fork < 0){
            printf("Nie mozna zforkowac\n");
            exit(-1);
        }
        else if(pid_fork == 0){        
            printf("Katalog: %s pid: %d\n", path, getpid());           
            int exec_status = execlp("ls", "ls", "-l", path, NULL);
            if (exec_status != 0){
                printf("Nie mozna uruchomic");
                exit(-1);
            }
            exit(exec_status);
        } 
        else wait(0);
    }
    return 0;
}

int main(int argc, char** argv){
    if (argc != 2){
        printf("Zla liczba argumentow!\n");
        return -1;
    }

    char* path = argv[1];
    nftw(path, find, 10, FTW_PHYS);

    return 0;
}