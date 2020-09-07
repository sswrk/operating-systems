#define LINE_LENGTH_LIMIT 1000
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv){
    if (argc != 2){
        printf("Niepoprawna liczba argumentow\n");
        return -1;
    }

    FILE *f = fopen(argv[1], "r");
    FILE *sorted_f = popen("sort", "w");
    char* ln = malloc(LINE_LENGTH_LIMIT);
    while(fgets(ln, LINE_LENGTH_LIMIT, f)!=NULL)
        fputs(ln, sorted_f);
    pclose(sorted_f);
    free(ln);
    return 0;
}