#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv){
    if (argc != 4){
        printf("Niepoprawna liczba argumentow\n");
        return -1;
    }

    char* pipe_path = argv[1];
    char* file_path = argv[2];
    int N = atoi(argv[3]);

    srand(time(NULL));

    FILE *pipe = fopen(pipe_path, "w");

    if (pipe == NULL){
        printf("Nie mozna otworzyc potoku nazwanego\n");
        return -1;
    }
    FILE *file = fopen(file_path, "r");

    if (file == NULL){
        printf("Nie mozna otworzyc pliku tekstowego\n");
        fclose(pipe);
        return -1;
    }

    char buf[N];

    while(fread(buf, 1, N, file) > 0){
        sleep(rand() % 2 + 1);
        char mes [N + 20];
        sprintf(mes, "#%d#%s\n", getpid(), buf);
        fwrite(mes, 1, strlen(mes), pipe);
    }

    fclose(pipe);
    fclose(file);
    return 0;
}