#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

clock_t t_begin, t_end;
struct tms tms_start, tms_end;
FILE *times_log;

void timerStart(){
    t_begin = times(&tms_start);
}
void timerStop(){
    t_end = times(&tms_end);
}

void printTimes(FILE* file, char* name){

    printf("Rodzaj operacji: %s\n", name);
    printf("Czas rzeczywisty:: %f\n", (double) (t_end - t_begin) / sysconf(_SC_CLK_TCK));
    printf("Czas uzytkownika: %f\n", (double) (tms_end.tms_utime - tms_start.tms_utime) / sysconf(_SC_CLK_TCK));
    printf("Czas systemowy: %f\n\n", (double)(tms_end.tms_stime - tms_start.tms_stime) / sysconf(_SC_CLK_TCK));

    fprintf(file, "Rodzaj operacji: %s\n", name);
    fprintf(file, "Czas rzeczywisty:: %f\n", (double) (t_end - t_begin) / sysconf(_SC_CLK_TCK));
    fprintf(file, "Czas uzytkownika: %f\n", (double) (tms_end.tms_utime - tms_start.tms_utime) / sysconf(_SC_CLK_TCK));
    fprintf(file, "Czas systemowy: %f\n\n", (double)(tms_end.tms_stime - tms_start.tms_stime) / sysconf(_SC_CLK_TCK));
}

void generate(char *path, int stringNumber, int length) {
    FILE *file = fopen(path, "w+");
    char *current_string = malloc(1 + length * sizeof(char));

    for (int i = 0; i < stringNumber; i++) {
        if(length>0)
            for (int j = 0; j < length; j++)
                current_string[j] = "1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"[rand() % 62];
        current_string[length] = '\n';
        fwrite(current_string, sizeof(char), (size_t) length + 1, file);
    }
    free(current_string);
    fclose(file);
};

//PRZY UZYCIU FUNKCJI BIBLIOTEKI C: fread i fwrite

//KOPIOWANIE

void copy_lib(char* src, char* destFile, int stringNumber, int length){
    FILE *srcFile = fopen(src, "r");
    if (srcFile == NULL){
        fprintf(stderr, "Nie mozna otworzyc pliku zrodlego\n");
        exit(-1);
    }
    FILE *dest = fopen(destFile, "w+");
    if (dest == NULL){
        fprintf(stderr, "Nie mozna otworzyc pliku docelego\n");
        exit(-1);
    }
    char *tmp_string = malloc(length * sizeof(char));

    for (int i = 0; i < stringNumber; i++){
        if (fread(tmp_string, sizeof(char), (size_t) (length + 1), srcFile) != length + 1){
            fprintf(stderr, "Nie udalo sie odczytac danych z pliku zrodlego\n");
            exit(-1);
        }
        if(fwrite(tmp_string, sizeof(char), (size_t)(length + 1), dest) != length + 1){
            fprintf(stderr, "Nie udalo sie wkleic danych do pliku docelego\n");
            exit(-1);
        }
    }
    fclose(srcFile);
    fclose(dest);
    free(tmp_string);
}

//QUICKSORT

void swap_lib(FILE * file, int i, int j, int length){
    char* str1 = calloc(length + 1, sizeof(char));
    char* str2 = calloc(length + 1, sizeof(char));

    fseek(file, (length + 1) * i, 0);
    fread(str1, sizeof(char), length + 1, file);

    fseek(file, (length + 1) * j, 0);
    fread(str2, sizeof(char), length + 1, file);

    fseek(file, (length + 1) * j, 0);
    fwrite(str1, sizeof(char), length + 1, file);

    fseek(file, (length + 1) * i, 0);
    fwrite(str2, sizeof(char), length + 1, file);

    free(str1);
    free(str2);
}

int partition_lib(FILE *file, int length, int l, int r){
    char* p = calloc(length + 1, sizeof(char));
    fseek(file, (length + 1) * r, 0);
    fread(p, sizeof(char), length + 1, file);
    int i = l - 1;
    char * tmp_string = calloc(length + 1, sizeof(char));
    for (int j = l; j < r; j ++){
        fseek(file, (length + 1) * j, 0);
        fread(tmp_string, sizeof(char), length + 1, file);
        if (strcmp(tmp_string, p) < 0){
            i++;
            swap_lib(file, i, j, length);
        }
    }
    swap_lib(file, i + 1, r, length);
    free(tmp_string);
    free(p);
    return (i + 1);
}

void quicksort_lib(FILE *file, int length,  int l, int r){
    if (l < r){
        int p = partition_lib(file, length, l, r);
        quicksort_lib(file, length, l, p - 1);
        quicksort_lib(file, length, p + 1, r);
    }
}

void sort_lib(char* file, int stringNumber, int length){
    FILE *srcFile = fopen(file, "r+");
    if (srcFile == NULL){
        fprintf(stderr, "Nie udalo sie otworzyc pliku\n");
        exit(-1);
    }
    quicksort_lib(srcFile, length, 0, stringNumber - 1);
    fclose(srcFile);
}

//PRZY UŻYCIU FUNKCJI SYSTEMOWYCH: read i write

//KOPIOWANIE

void copy_sys(char* src, char* destFile, int stringNumber, int length){
    int srcFile = open(src, O_RDONLY);
    if (srcFile < 0){
        fprintf(stderr, "Nie mozna otworzyc pliku zrodlego\n");
        exit(-1);
    }
    int dest = open(destFile, O_CREAT | O_WRONLY);
    if (dest < 0){
        fprintf(stderr, "Nie mozna otworzyc pliku docelego\n");
        exit(-1);
    }
    char *tmp_string = malloc(length * sizeof(char));

    for (int i = 0; i < stringNumber; i++){
        if (read(srcFile, tmp_string, length + 1) < 0){
            fprintf(stderr, "Nie udalo sie odczytac danych z pliku zrodlego\n");
            exit(-1);
        }
        if (write(dest, tmp_string, length + 1) < 0){
            fprintf(stderr, "Nie udalo sie wkleic danych do pliku docelego\n");
            exit(-1);
        }
    }
    close(srcFile);
    close(dest);
    free(tmp_string);
}

//QUICKSORT

void swap_sys(int file, int i, int j, int length){
    char* str1 = calloc(length + 1, sizeof(char));
    char* str2 = calloc(length + 1, sizeof(char));

    lseek(file, (length + 1) * i, 0);
    read(file, str1, length + 1);

    lseek(file, (length + 1) * j, 0);
    read(file, str2, length + 1);

    lseek(file, (length + 1) * j, 0);
    write(file, str1, length + 1);

    lseek(file, (length + 1) * i, 0);
    write(file, str2, length + 1);

    free(str1);
    free(str2);
}

int partition_sys(int file, int length, int l, int r){
    char* p = calloc(length + 1, sizeof(char));
    lseek(file, (length + 1) * r, 0);
    read(file, p, length + 1);
    
    int i = l - 1;
    char * tmp_string = calloc(length + 1, sizeof(char));
    for (int j = l; j < r; j ++){
        lseek(file, (length + 1) * j, 0);
        read(file, tmp_string, length + 1);
        if (strcmp(tmp_string, p) < 0){
            i++;
            swap_sys(file, i, j, length);
        }
    }
    swap_sys(file, i + 1, r, length);
    free(tmp_string);
    free(p);
    return (i + 1);
}

void quicksort_sys(int file, int length,  int l, int r){
    if (l < r){
        int p = partition_sys(file, length, l, r);
        quicksort_sys(file, length, l, p - 1);
        quicksort_sys(file, length, p + 1, r);
    }
}

void sort_sys(char* file, int stringNumber, int length){
    int srcFile = open(file, O_RDWR);
    if (srcFile < 0){
        fprintf(stderr, "Nie udalo sie otworzyc pliku\n");
        exit(-1);
    }
    quicksort_sys(srcFile, length, 0, stringNumber - 1);
    close(srcFile);

}

int main(int argc, char** argv){

    char* file = "wyniki.txt";
    times_log = fopen(file, "a");
    if (times_log == NULL){
        return -1;
    }
    int i = 1;
    while (i < argc){
        //GENERACJA
        if (!strcmp(argv[i], "generate")){
            if (i + 3 > argc){
                printf("Niepoprawne arumenty!");
                return -1;
            }
            file = argv[i + 1];
            int stringNumber = atoi(argv[i + 2]);
            int length = atoi(argv[i + 3]);
            generate(file, stringNumber, length);
            
            i += 4;
        }
        //SORTOWANIE
        else if (!strcmp(argv[i], "sort")){
            if (i + 4 >= argc){
                printf("Niepoprawne arumenty!");
                return -1;
            }
            file = argv[i + 1];
            int stringNumber = atoi(argv[i + 2]);
            int length = atoi(argv[i + 3]);
            char* operationlib_type = argv[i + 4];
            char buffer[64];
            snprintf(buffer, sizeof buffer,"sortowanie %s, ilosc: %d, bajty: %d", operationlib_type, stringNumber, length);
            
            if (!strcmp(operationlib_type, "lib")){
                timerStart();
                sort_lib(file, stringNumber, length);
                timerStop();
                printTimes(times_log, buffer);
            }
            else if (!strcmp(operationlib_type, "sys")){
                timerStart();
                sort_sys(file, stringNumber, length);
                timerStop();
                printTimes(times_log, buffer);
            }
            else{
                printf("Zly argument!");
                return -1;
            }

            i += 5;
        }
        //KOPIOWANIE
        else if (!strcmp(argv[i], "copy")){
            if (i + 5 >= argc){
                printf("Niepoprawne argumenty!");
                return -1;
            }
            char* srcFile = argv[i + 1];
            char* dest = argv[i + 2];
            int stringNumber = atoi(argv[i + 3]);
            int length = atoi(argv[i + 4]);
            char* operationlib_type = argv[i + 5];

            char operation[64];
            snprintf(operation, sizeof operation,"copy %s, ilosc: %d, bajty: %d", operationlib_type, stringNumber, length);

            if (!strcmp(operationlib_type, "lib")){
                timerStart();
                copy_lib(srcFile, dest, stringNumber, length);
                timerStop();
                printTimes(times_log, operation);
            }
            else if (!strcmp(operationlib_type, "sys")){
                timerStart();
                copy_sys(srcFile, dest, stringNumber, length);
                timerStop();
                printTimes(times_log, operation);
            }

            i += 6;
        }

        else{
            printf("Niepoprawna komenda!");
            return -1;
        }
    }
    fclose(times_log);
    return 0;
}