#ifndef library_h
#define library_h

#include <stdio.h>

struct block {
    int block_size;
    int count;
    char** operation_array;
};

struct array {
    int array_size;
    int count;
    struct block** block_array;
};

struct array* createArray(int array_size);

struct block* createBlock(int block_size);

void deleteOperation(struct array* arr, int block_index, int op_index);

void deleteBlock(struct array* arr, int block_index);

FILE* compareFiles(char* text1, char* text2);

void saveComparison(struct array* main_array, FILE* result_file);

int getBlockOperationsNumber(struct array* arr, int block_index);
#endif