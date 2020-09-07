#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/times.h>

#include <ctype.h>
#include <time.h>

#include <dlfcn.h>

#include "library.h"

double time_count(clock_t time1, clock_t time2){ return ((double)(time2 - time1) / CLOCKS_PER_SEC); }

int main(int argc, char** argv) {

    if(argc <= 1) {
        printf("create_table size - tworzy tworzy nowa tablice\ncompare_pairs file1:file2 .. - porownywanie plikow\nremove_block index - usuń blok\nremove_operation block_index operation_index - usun operacje\n");
        return 0;
    }

    struct array* main_array;

    struct tms* t_before = malloc(sizeof(struct tms*)), *t_after = malloc(sizeof(struct tms*));
    clock_t before = 0, after = 0;

    main_array = createArray(atoi(argv[1]));

    before = times(t_before);
    for(int i = 2; i < argc; ) {

        if (strcmp(argv[i], "compare_pairs") == 0) {
            i++;
            while(i < argc && strchr(argv[i], ':') != NULL) {
                char* txtfile1 = strtok(argv[i], ":");
                char* txtfile2 = strtok(NULL, ":");
                i++;

                FILE* tmp_file = compareFiles(txtfile1, txtfile2);
                saveComparison(main_array, tmp_file);
            }

        } else if (strcmp(argv[i], "remove_operation") == 0) {
            int block_index = atoi(argv[i + 1]), operation_index = atoi(argv[i + 2]);
            i += 3;

            deleteOperation(main_array, block_index, operation_index);

        } else if (strcmp(argv[i], "remove_block") == 0) {
            int block_index = atoi(argv[i + 1]);
            i += 2;

            deleteBlock(main_array, block_index);

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