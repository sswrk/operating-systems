#define ABS_MAX 128
#define ROW_LENGTH_MAX 1024
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/times.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <dirent.h>
#include <limits.h>
#include <stdbool.h>
#include <ftw.h>
#include <math.h>
#include <sys/time.h>
#include <sys/resource.h>

struct tms tms_start, tms_stop;
clock_t start_t, stop_t;

int min(int a, int b){
    if(a < b) return a;
    return b;
}

typedef struct{
    char* path;
    int rows;
    int columns;
    int column_width;
} Matrix;

Matrix **matrix_A;
Matrix **matrix_B;
Matrix **matrix_C;

int pairs;

Matrix *matrixRead(char *path){
    Matrix *result = malloc(sizeof(Matrix));
    result->path = calloc(FILENAME_MAX, sizeof(char));
    strcpy(result->path, path);
    FILE *f = fopen(path, "r");
    if (f == NULL){
        printf("Nie mozna otworzyc %s\n", path);
        exit(-1);
    }
    int counter = 0;
    fseek(f, 0, 0);
    char *buffer = NULL;
    size_t length = 0;

    while(getline(&buffer, &length, f) != -1 && buffer[0] != ' ' && buffer[0] != '\n') counter++;
    result->rows = counter;
    counter = 1;
    fseek(f, 0, 0);
    for(char i = getc(f); i != '\n'; i = getc(f))
        if (i == ' ') counter++;
    result->columns = counter;

    fclose(f);
    return result;
}

Matrix* generateResultMatrix(char *path, int rows, int columns){
    Matrix *result = malloc(sizeof(Matrix));
    result->path = calloc(FILENAME_MAX, sizeof(char));
    strcpy(result->path, path);
    result->rows = rows;
    result->columns = columns;
    result->column_width = (int) log10(ABS_MAX * ABS_MAX) + 3;
    FILE *f = fopen(result->path, "w+");
    for (int i = 0; i < result->rows; i++){
        for (int j = 0; j < result->column_width * result->columns - 1; j++)
            fwrite(" ", 1, 1, f);
        fwrite("\n", 1, 1, f);
    }
    fclose(f);
    return result;
}

int getValue(Matrix *matrix, int row, int column, FILE *f){
    if (row >= matrix->rows || column >= matrix->columns){
        printf("Wyjscie poza zakres macierzy\n");
        exit(-1);
    }
    fseek(f, 0, 0);
    int counter = 0;
    char* buffer = calloc(ROW_LENGTH_MAX, sizeof(char));

    while(fgets(buffer, ROW_LENGTH_MAX, f) != NULL && counter < matrix->rows && counter < row) counter++;
    counter = 0;
    char *tok = strtok(buffer, " ");

    while (tok != NULL && counter < matrix->columns && counter < column){
        tok = strtok(NULL, " ");
        counter++;
    }
    int result;
    if (tok == NULL){
        printf("Wyjscie poza zakres macierzy w pliku");
        exit(-1);
    }
    result = atoi(tok);
    free(buffer);
    return result;
}

void setValue(Matrix *matrix, int row, int column, FILE *f, int val){
    fseek(f, 0, 0);
    int index = matrix->column_width * matrix->columns * row + column * matrix->column_width;
    char *num = (char*)calloc(matrix->column_width, sizeof(char));
    sprintf(num, "%d", val);
    int j = matrix->column_width - 1;
    while(num[j] == 0){
        num[j]=' ';
        j--;
    }
    fseek(f, index, 0);
    fwrite(num, sizeof(char), matrix->column_width - 1, f);
    free(num);

}

void columnMultiply(Matrix *A, Matrix *B, Matrix*C, FILE *file_A, FILE *file_B, FILE *file_C, int column){
    fseek(file_A, 0, 0);
    fseek(file_B, 0, 0);
    fseek(file_C, 0, 0);

    for (int i = 0; i < A->rows; i++){
        int result = 0;
        for (int j = 0; j < A->columns; j++){
            result += getValue(A, i, j, file_A) * getValue(B, j, column, file_B);
        }
        setValue(C, i, column, file_C, result);
    }
}

char *filename(int i, int process){
    char* path = calloc(FILENAME_MAX, sizeof(char));
    char num[3];
    sprintf(num, "%d", process);
    strcpy(path, matrix_C[i]->path);
    strcat(path, "_");
    strcat(path, num);
    return path;
}

void matrixesMultiply(int process, int processes, int time_limit, int distinct, int *num_operations){
    start_t = times(&tms_start);
    *num_operations = 0;

    for (int i = 0; i < pairs; i++){
        int process_columns;
        int column_zero;
        if (process >= matrix_B[i]->columns) continue;

        if (processes > matrix_B[i]->columns){
            process_columns = 1;
            column_zero = process;
        }
        else {
            process_columns = matrix_B[i]->columns / processes;
            column_zero = process_columns * process;
            if (process == processes - 1){
                process_columns += matrix_B[i]->columns - process_columns * (process + 1);
            }
        }


        if (distinct == true){
            FILE * A = fopen(matrix_A[i]->path, "r");
            FILE * B = fopen(matrix_B[i]->path, "r");
            if (A == NULL || B == NULL){
                printf("Nie mozna otworzyc pliku\n");
                exit(-1);
            }

            char*path = filename(i, process);
            Matrix *newMatrix = generateResultMatrix(path, matrix_C[i]->rows, process_columns);
            FILE * f = fopen(path, "r+");

            for (int i = 0; i < process_columns; i++){
                for (int j = 0; j < matrix_A[i]->rows; j++){
                    int result = 0;
                    for (int k = 0; k < matrix_A[i]->columns; k++){
                        result += getValue(matrix_A[i], j, k, A) * getValue(matrix_B[i], k, column_zero + i, B);
                    }
                    setValue(newMatrix, j, i, f, result);
                }
            }
            fclose(A);
            fclose(B);
            fclose(f);
            free(newMatrix->path);
            free(newMatrix);
            free(path);
        }
        else{
            FILE *A = fopen(matrix_A[i]->path, "r");
            FILE *B = fopen(matrix_B[i]->path, "r");
            FILE *C = fopen(matrix_C[i]->path, "r+");
            if (A == NULL || B == NULL || C == NULL){
                printf("Nie mozna otworzyc pliku\n");
                exit(-1);
            }
            flock(fileno(C), LOCK_EX);
            for (int j = column_zero; j < column_zero + process_columns; j++)
                columnMultiply(matrix_A[i], matrix_B[i], matrix_C[i], A, B, C, j);
            flock(fileno(C), LOCK_UN);
            fclose(A);
            fclose(B);
            fclose(C);
        }
        (*num_operations)++;
        stop_t = times(&tms_stop);
        if ((int) ((start_t - stop_t) / sysconf(_SC_CLK_TCK)) >= time_limit) break;
    }
}

int main(int argc, char** argv){
    if (argc != 7){
        printf("Zla liczba argumentow");
        return -1;
    }
    int processes = atoi(argv[2]);
    if (processes <= 0){
        printf("Liczba procesow musi byc dodatnia\n");
        return -1;
    }
    int time_limit = atoi(argv[3]);
    bool distinct = false;

    if (!strcmp("distinct", argv[4]))
        distinct = true;
    
    int hardTime = atoi(argv[5]);

    if (hardTime <= 0){
        printf("Twardy limit czasy musi byc dodatni\n");
        return -1;
    }

    int hardMemory = atoi(argv[6]);

    if (hardMemory <= 0){
        printf("Twardy limit pamieci musi byc dodatni\n");
        return -1;
    }

    FILE *f = fopen(argv[1], "r");
    if (f == NULL){
        printf("Nie mozna otworzyc listy\n");
        exit(-1); 
    }

    char*buffer = calloc(ROW_LENGTH_MAX, sizeof(char));
    int i = 0;
    while(fgets(buffer, ROW_LENGTH_MAX, f) != NULL) i++;
    matrix_A = (Matrix**) calloc(i, sizeof(Matrix*));
    matrix_B = (Matrix**) calloc(i, sizeof(Matrix*));
    matrix_C = (Matrix**) calloc(i, sizeof(Matrix*));
    pairs = i;
    fseek(f, 0, 0);
    i = 0;
    while(fgets(buffer, ROW_LENGTH_MAX, f) != NULL){
        char* A = strtok(buffer, " ");
        char* B = strtok(NULL, " ");
        char* C = strtok(NULL, "\n");
        matrix_A[i] = matrixRead(A);
        matrix_B[i] = matrixRead(B);
        matrix_C[i] = generateResultMatrix(C, matrix_A[i]->rows, matrix_B[i]->columns);
        i++;
    }
    free(buffer);
    fclose(f);

    int *matrix_processes = NULL;

    if (distinct == true){
        matrix_processes = calloc(pairs, sizeof(int));
        for (int i = 0; i < pairs; i++)
            matrix_processes[i] = min(processes, matrix_B[i]->columns);
        for (int i = 0; i < pairs; i++){
            for (int j = 0; j < matrix_processes[i]; j++){
                char* f = filename(i, matrix_processes[i]);
                FILE * ff = fopen(f, "w+");
                fclose(ff);
                free(f);
            }
        }
    }

    pid_t *child_processes = calloc(processes, sizeof(pid_t));

    for (int i = 0; i < processes; i++){
        pid_t child_process = fork();
        if (child_process == 0){
            int num_operations;
            struct rlimit* timelimit = malloc(sizeof(struct rlimit));
            timelimit -> rlim_cur = hardTime;
            timelimit -> rlim_max = hardTime;
            setrlimit(RLIMIT_CPU, timelimit);
            struct rlimit* memory_limit = malloc(sizeof(struct rlimit));
            memory_limit -> rlim_cur = hardMemory * (1 << 20);
            memory_limit -> rlim_max = hardMemory * (1 << 20);
            setrlimit(RLIMIT_AS, memory_limit);
            matrixesMultiply(i, processes, time_limit, distinct, &num_operations);
            free(timelimit);
            free(memory_limit);
            exit(num_operations);
        }
        else if (child_process > 0) child_processes[i] = child_process;
        else exit(-1);
    }
    if (distinct == true){
        for (int i = 0; i < processes; i++) wait(0);

        for(int i = 0; i < pairs; i++){
            char** arguments = calloc(matrix_processes[i] + 2, sizeof(char*));
            arguments[0] = (char*) calloc(6, sizeof(char));
            strcpy(arguments[0], "paste");

            for (int j = 0; j < matrix_processes[i]; j++){
                arguments[j + 1] = calloc(FILENAME_MAX, sizeof(char));
                strcpy(arguments[j + 1], filename(i, j));
            }
            arguments[matrix_processes[i] + 1]=NULL;

            pid_t child_process = fork();
            if (child_process == 0){
                int fd = open(matrix_C[i]->path, O_WRONLY | O_CREAT | O_TRUNC, 0);
                dup2(fd,1);
                close(fd);
                execvp("paste", arguments);
                strcpy(arguments[0], "rm");
            }
            else wait(0);

            pid_t child_process2 = fork();
            if (child_process2 == 0){
                strcpy(arguments[0], "rm");
                execvp("rm", arguments);
            }
            else wait(0);

            for (int j = 0; j < matrix_processes[i] + 1; j++) free(arguments[j]);
        }
    }

    struct rusage* usage = malloc(sizeof(struct rusage));
    for (int i = 0; i < processes; i++){
        int status;
        waitpid(child_processes[i], &status, 0);
        printf("Proces PID = %d wykonal %d operacji mnozenia\n", child_processes[i], WEXITSTATUS(status));
        if (getrusage(child_processes[i], usage) == RUSAGE_CHILDREN){
            double user_time = usage -> ru_utime.tv_sec + 0.000001f * usage -> ru_utime.tv_usec;
            double system_time = usage -> ru_stime.tv_sec + 0.000001f * usage -> ru_stime.tv_usec;
            printf("Czas uzytkownika: %f Czas systemowy: %f\n\n", user_time, system_time);
        }
    }
    free(usage);

    for (int i = 0; i < pairs; i++){
        free(matrix_A[i]->path);
        free(matrix_B[i]->path);
        free(matrix_C[i]->path);
        free(matrix_A[i]);
        free(matrix_B[i]);
        free(matrix_C[i]);
    }
    free(matrix_A);
    free(matrix_B);
    free(matrix_C);

    return 0;
}