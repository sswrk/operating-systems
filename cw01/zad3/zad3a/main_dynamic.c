#include <dlfcn.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "library.h"
#include <unistd.h>
#include <sys/times.h>
#include <dlfcn.h>
#include <ctype.h>
#include <time.h>

double time_count(clock_t t1, clock_t t2){ return ((double)(t2 - t1) / CLOCKS_PER_SEC); }

int main(int argc, char** argv) {
    struct array* main_array;

    if(argc <= 1) {
        printf("create_table size - tworzy tworzy nowa tablice\ncompare_pairs file1:file2 .. - porownywanie plikow\nremove_block index - usuń blok\nremove_operation block_index operation_index - usun operacje\n");
        return 0;
    }

    void* dl_handle = dlopen("./library.so", RTLD_LAZY);
    if(!dl_handle) {
        printf("Blad przy ladowaniu biblioteki\n");
        return 1;
    }

    struct array* (*dl_createArray)(int);
    dl_createArray = (struct array* (*)(int))dlsym(dl_handle, "createArray");

    FILE* (*dl_compareFiles)(char*, char*);
    dl_compareFiles = (FILE* (*)(char*, char*))dlsym(dl_handle, "compareFiles");

    void (*dl_deleteOperation)(struct array*, int, int);
    dl_deleteOperation = (void (*)(struct array*, int, int))dlsym(dl_handle, "deleteOperation");

    void (*dl_deleteBlock)(struct array*, int);
    dl_deleteBlock = (void (*)(struct array*, int))dlsym(dl_handle, "deleteBlock");

   void (*dl_saveComparison)(struct array*, FILE*);
    dl_saveComparison = (void (*)(struct array*, FILE*))dlsym(dl_handle, "saveComparison");


    struct tms* t_before = malloc(sizeof(struct tms*)), *t_after = malloc(sizeof(struct tms*));
    clock_t before = 0, after = 0;

    main_array = dl_createArray(atoi(argv[1]));

    before = times(t_before);
    for(int i = 2; i < argc; ) {

        if (strcmp(argv[i], "compare_pairs") == 0) {
            i++;
            while(i < argc && strchr(argv[i], ':') != NULL) {
                char* text1 = strtok(argv[i], ":");
                char* text2 = strtok(NULL, ":");
                FILE* tmp_file = dl_compareFiles(text1, text2);
                dl_saveComparison(main_array, tmp_file);
                i++;
            }
        } else if (strcmp(argv[i], "remove_operation") == 0) {
            int block_index = atoi(argv[i + 1]), operation_index = atoi(argv[i + 2]);
            dl_deleteOperation(main_array, block_index, operation_index);
            i += 3;
        } else if (strcmp(argv[i], "remove_block") == 0) {
            int block_index = atoi(argv[i + 1]);
            dl_deleteBlock(main_array, block_index);
            i += 2;
        } else {
            printf("Zly argument\n");
            return 1;
        }

    }
    after = times(t_after);
    printf("Czas systemowy: %f\n", time_count(t_before->tms_stime, t_after->tms_stime));
    printf("Czas użytkownika: %f\n", time_count(t_before->tms_utime, t_after->tms_utime));
    printf("Czas rzeczywisty: %f\n", time_count(before,after));  

    return 0;
}