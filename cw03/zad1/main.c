#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <dirent.h>
#include <stdbool.h>
#include <wait.h>
#include <unistd.h>

void find(char *path){
    if (path == NULL) return;
    DIR* dir = opendir(path);
    if (dir == NULL){
        printf("Nie mozna otworzyc katalogu\n");
        exit(-1);
    }
    struct stat buffer;
    lstat(path, &buffer);
    if (S_ISDIR(buffer.st_mode)){
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

        else{
            wait(0);
        }
    }
    
    
    struct dirent *file;
    char new_path[256];
    while ((file = readdir(dir)) != NULL){
        strcpy(new_path, path);
        strcat(new_path, "/");
        strcat(new_path, file -> d_name);

        if (lstat(new_path, &buffer) < 0){
            printf("Nie mozna wykonac lstat dla pliku %s: ", new_path);
            exit(-1);
        }
        
        if (S_ISDIR(buffer.st_mode)){

            if (strcmp(file -> d_name, ".") == 0 || strcmp(file -> d_name, "..") == 0){
                continue;
            }
            find(new_path);
        }
    }
    closedir(dir);
}

int main(int argc, char** argv){
    if (argc != 2){
        printf("Zla liczba argumentow!\n");
        return -1;
    }

    char* path = argv[1];
    find(path);

    return 0;
}