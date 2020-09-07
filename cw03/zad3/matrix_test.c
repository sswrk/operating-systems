#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct Matrix{
    int **values;
    int columns;
    int rows;
} Matrix;

Matrix* matrix_read(char*path){
	FILE *file = fopen(path, "r");
    if (file == NULL){
        printf("Nie mozna otworzyc %s\n", path);
        exit(-1);
    }
    Matrix* result = malloc(sizeof(Matrix));
    int rows = 1;
	int columns = 1;
	char buf[1024];
	char* values;
	char lim[] = " \t\n";

	if(fgets(buf, sizeof buf, file)){
		strtok(buf, lim);
		while (strtok(NULL, lim) != NULL) columns++;
	}
	while (fgets(buf, sizeof buf, file)) rows++;
	rewind(file);

	result->rows = rows;
	result->columns = columns;
	result->values = calloc(rows, sizeof(int*));
	
    for(int i = 0; i<rows; i++) result->values[i] = calloc(columns, sizeof(int));
	
    int i = 0;
	while (fgets(buf, sizeof buf, file)){
		int j = 0;
		values = strtok(buf, lim);
		result->values[i][j] = atoi(values);
		while (values != NULL && j < columns){
			result ->values[i][j] = atoi(values);
			values = strtok(NULL, lim);
			j++;
		}
		i++;
	}
	fclose(file);
	return result;
}

Matrix *multiply(Matrix *matrix_A, Matrix *matrix_B){
    Matrix *matrix_C = (Matrix *) malloc(sizeof(Matrix));
    matrix_C->columns = matrix_B->columns;
    matrix_C->rows = matrix_A->rows;
    matrix_C->values = (int **)calloc(matrix_C->rows, sizeof(int *));
    for (int i = 0; i < matrix_C->rows; i ++)
        matrix_C->values[i] = (int *)calloc(matrix_C->columns, sizeof(int));
    for (int i = 0;  i< matrix_C->rows; i ++){
        for (int j = 0; j < matrix_C->columns; j ++){
            matrix_C->values[i][j] = 0;
            for (int k = 0; k < matrix_A->columns; k ++){
                matrix_C->values[i][j] += matrix_A->values[i][k] * matrix_B->values[k][j];
            }
        }
    }
    return matrix_C;
}

bool check(Matrix *C1, Matrix * C2){
    if (C1->columns != C2->columns || C1->rows != C2->rows) 
        return false;

    for (int i = 0; i < C1->rows; i ++)
        for (int j = 0; j < C1->columns; j ++)
            if (C1->values[i][j] != C2->values[i][j]) return false;
    return true;
}

int main(int argc, char**argv){
    if (argc < 2){
        printf("Zla liczba argumentow\n");
        return -1;
    }

    FILE *lista = fopen(argv[1], "r");
    if (lista == NULL){
        printf("Nie mozna otworzyc listy: %s\n", argv[1]);
        return -1;
    }

    char file[1024];
    int i = 0;
    Matrix *matrix_A, *matrix_B, *matrix_C;

    while(fscanf(lista, "%s", file) != EOF){
        if (i % 3 == 0) matrix_A = matrix_read(file);
        else if (i % 3 == 1) matrix_B = matrix_read(file);
        else{
            matrix_C = matrix_read(file);
            if(check(multiply(matrix_A, matrix_B), matrix_C)) printf("TEST %d OK\n", i/3);
            else printf("TEST %d ZLE\n", i/3);
            for (int r = 0; r < matrix_A->rows; r++) free(matrix_A->values[r]);
            free(matrix_A->values);
            free(matrix_A);
            for (int r = 0; r < matrix_B->rows; r++) free(matrix_B->values[r]);
            free(matrix_B->values);
            free(matrix_B);
            for (int r = 0; r < matrix_C->rows; r++) free(matrix_C->values[r]);
            free(matrix_C->values);
            free(matrix_C);
        }
        i++;
    }
    return 0;

}