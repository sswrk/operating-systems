#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int minval = -128;
int maxval = 128;

void matrixesGenerate(int pairs, int min, int max){

    FILE *lista = fopen("lista", "w+");
    if(lista == NULL)
        return;
    char* file = (char*) calloc(100, sizeof(char));

    srand(time(NULL));

    for (int i = 0; i < pairs; i++){
        char num[3];
        sprintf(num, "%d", i);

        strcpy(file, "matrixes/");
        strcat(file, "A");
        strcat(file, num);
        FILE *file_A = fopen(file, "w+");
        fprintf(lista, "%s ", file);
        strcpy(file, "matrixes/");
        strcat(file, "B");
        strcat(file, num);
        FILE *file_B = fopen(file, "w+");
        fprintf(lista, "%s ", file);
        strcpy(file, "matrixes/");
        strcat(file, "C");
        strcat(file, num);
        fprintf(lista, "%s\n", file);

        int A_rows = rand() % (max - min + 1) + min;
        int A_columns = rand() % (max - min + 1) + min;
        int B_columns = rand() % (max - min + 1) + min;
        
        for (int i = 0; i < A_rows; i++){
            for (int j = 0; j < A_columns; j++){
                fprintf(file_A, "%d", rand() % (maxval - minval + 1) + minval);
                if (j != A_columns - 1) fprintf(file_A, " ");
            }
            fprintf(file_A, "\n");
        }

        for (int i = 0; i < A_columns; i++){
            for (int j = 0; j < B_columns; j++){
                fprintf(file_B, "%d", rand() % (maxval - minval + 1) + minval);
                if (j != B_columns - 1) fprintf(file_B, " ");
            }
            fprintf(file_B, "\n");
        }

        fclose(file_A);
        fclose(file_B);
    }
}

int main(int argc, char **argv){
    if (argc != 4){
        printf("Nieprawidlowa liczba argumentow\n");
        return -1;
    }
    int pairs = atoi(argv[1]);
    int min = atoi(argv[2]);
    int max = atoi(argv[3]);

    if (pairs <= 0 || min <= 0 || max <= 0){
        printf("Argumenty musza byc dodatnie\n");
        return -1;
    }

    if (min > max){
        printf("Wartosc minimalna musi byc mniejsza niz wartosc maksymalna\n");
        return -1;
    }

    matrixesGenerate(pairs, min, max);
    return 0;
}