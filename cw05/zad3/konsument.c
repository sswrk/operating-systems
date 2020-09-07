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

    FILE *pipe = fopen(pipe_path, "r");

    if (pipe == NULL){
        printf("Nie mozna otworzyc potoku nazwanego\n");
        return -1;
    }

    FILE *file = fopen(file_path, "w");

    if (file == NULL){
        printf("Nie mozna otworzyc pliku tekstowego\n");
        fclose(pipe);
        return -1;
    }

    char buf[N];

    while(fgets(buf, N, pipe) != NULL)
        fprintf(file, buf, strlen(buf));

    fclose(pipe);
    fclose(file);
    return 0;
}