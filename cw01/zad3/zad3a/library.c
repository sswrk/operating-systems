#define MAX_COMMAND_LENGTH 1000
#define MAX_OPERATIONS 100
#define MAX_LINE_LENGTH 1000

#include "library.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct array* createArray(int array_size){
    struct array* result = malloc(sizeof(struct array));
    result->array_size = array_size;
    result->count = 0;
    result->block_array = (struct block**) calloc(array_size, sizeof(struct block*));
    return result;
}

struct block* createBlock(int block_size){
    struct block* result = malloc(sizeof(struct block));
    result->block_size = block_size;
    result->count = 0;
    result->operation_array = (char**) calloc(block_size, sizeof(char*));
    return result;
}

int getBlockOperationsNumber(struct array* arr, int block_index){
    return arr->block_array[block_index]->count;
}

void deleteOperation(struct array* arr, int block_index, int op_index){
    struct block* b = arr->block_array[block_index];
    free(b->operation_array[op_index]);
    for(int i = op_index; i < b->count - 1; i++){
        b->operation_array[i] = b->operation_array[i + 1];
    }
    b->count--;
}

void deleteBlock(struct array* arr, int block_index){
    for(int i = 0; i < arr->block_array[block_index]->count; i++){
        free(arr->block_array[block_index]->operation_array[i]);
    }
    free(arr->block_array[block_index]);
    for(int i = block_index; i < arr->count - 1; i++){
        arr->block_array[i] = arr->block_array[i + 1];
    }
    arr->count--;
}

FILE* compareFiles(char* text1, char* text2){
    char command[MAX_COMMAND_LENGTH];

    strcpy(command, "diff ");
    strcat(command, text1);
    strcat(command, " ");
    strcat(command, text2);
    strcat(command, " > temporary_file.txt");
    system(command);

    FILE* result = fopen("temporary_file.txt", "r");

    return result;
}

void saveComparison(struct array* main_array, FILE* temporary_file){
    char* buffer = NULL;
    size_t len = 0;

    struct block* b = createBlock(MAX_OPERATIONS);
    b->count = -1;

    while(getline(&buffer, &len, temporary_file) != -1){
        if(buffer[0] == '<' || buffer[0] == '-' || buffer[0] == '>'){
            strcat(b->operation_array[b->count], buffer);
        } else {
            b->operation_array[++b->count] = (char*) calloc(MAX_LINE_LENGTH, sizeof(char));
            strcpy(b->operation_array[b->count], buffer);
        }
    }
    b->count++;

    main_array->block_array[main_array->count++] = b;

    fclose(temporary_file);
}