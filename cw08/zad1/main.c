#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <math.h>
#include <sys/time.h>

int width;
int height;
const int LINELEN_LIMIT = 2048;
const int PIXEL_NUMBER = 512;
int threads_number;
int **img;
int **histogram_array;
typedef enum Div_method{ SIGN, BLOCK, INTERLEAVED } Div_method;

void image_load(char *file_input){
    FILE *file = fopen(file_input, "r");
    if(file==NULL){
        printf("Nie mozna otworzyc pliku\n");
        exit(-1);
    }
    char buf[LINELEN_LIMIT];
    int line_number = 0;
    int img_maxval;

    while(line_number<3 && fgets(buf, LINELEN_LIMIT, file)!=NULL){
        if(buf[0]=='#') continue;
        if(line_number==0 && strncmp("P2", buf, 2)){
            fclose(file);
            printf("Niepoprawne kolory\n");
            exit(-1);
        }
        else if(line_number==1){
            if(sscanf(buf, "%d %d\n", &width, &height)!=2){
                fclose(file);
                printf("Nie mozna odczytac rozmiarow obrazu\n");
                exit(-1);
            }
            img = (int **) calloc(height, sizeof(int *));
            for(int i=0; i<height; i++)
                img[i] =(int *) calloc(width, sizeof(int));
        }
        else if(line_number==2){
            if(sscanf(buf, "%d\n", &img_maxval)!=1){
                fclose(file);
                printf("Nie udalo sie odczytac maksymalnej wartosci\n");
                exit(-1);
            }
        }
        line_number++;
    }

    if(line_number!=3){
        fclose(file);
        printf("Nie udalo sie odczytac obrazu");
        exit(-1);
    }

    for(int i=0; i<height; i++){
        for(int j=0; j<width; j++){
            int value;
            fscanf(file, "%d", &value);
            img[i][j]=value;
        }
    }
    fclose(file);
}

int get_time(struct timeval time_start){
    struct timeval time_end;
    gettimeofday(&time_end, NULL);
    return((time_end.tv_sec - time_start.tv_sec) * 1e6) +(time_end.tv_usec - time_start.tv_usec);
}

void *sign(void *arg){
    struct timeval time_start;
    gettimeofday(&time_start, NULL);
    int index = *((int*)arg);

    int minval = index * ceil((double) PIXEL_NUMBER / threads_number);
    int maxval =(index + 1) * ceil((double) PIXEL_NUMBER / threads_number) - 1;

    if(maxval>PIXEL_NUMBER - 1)
        maxval = PIXEL_NUMBER - 1;
    
    for(int i = 0; i<height; i++){
        for(int j = 0; j<width; j++){
            if(img[i][j] >= minval && img[i][j]<=maxval)
                histogram_array[index][img[i][j]]++;
        }
    }

    int* result =(int*)malloc(sizeof(int));
    *result = get_time(time_start);
    return result;
}

void *block(void *arg){
    int index = *((int*)arg);

    struct timeval time_start;
    gettimeofday(&time_start, NULL);

    int col_start = index * ceil((double) width / threads_number);
    int col_end =(index + 1) * ceil((double) width / threads_number);
    if(col_end>width)
        col_end = width;

    for(int i = 0; i<height; i++)
        for(int j = col_start; j<col_end; j++)
            histogram_array[index][img[i][j]]++;

    int* result =(int*)malloc(sizeof(int));
    *result = get_time(time_start);
    return result;
}

void *interleaved(void *arg){
    struct timeval time_start;
    gettimeofday(&time_start, NULL);
    int index = *((int*)arg);

    int col_start = index;
    int step = threads_number;

    for(int i=0; i<height; i++)
        for(int j=col_start; j<width; j+=step)
            histogram_array[index][img[i][j]]++;

    int* result =(int*)malloc(sizeof(int));
    *result = get_time(time_start);
    return result;
}

void to_file(char* file_input){
    FILE *file = fopen(file_input, "w+");
    if(file==NULL){
        for(int i=0; i<height; i++)
            free(img[i]);
        free(img);
        for(int i=0; i<threads_number; i++)
            free(histogram_array[i]);
        free(histogram_array);
        printf("Nie udalo sie zapisac wyniku do pliku\n");
        exit(-1);
    }
    for(int i=0; i<PIXEL_NUMBER; i++){
        int value = 0;
        for(int j=0; j<threads_number; j++)
            value += histogram_array[j][i];
        fprintf(file, "%d: %d\n", i, value);
    }
    fclose(file);
}

void times_print(long int thread, int microseconds, FILE *times, int i){
    int seconds = microseconds/(double)(1000000.0);
    microseconds -= seconds*(double)(1000000.0);
    int miliseconds = microseconds/(double)(1000.0);
    microseconds -= miliseconds*(double)(1000.0);

    if(thread==-1){
        printf("Pelny czas: %d sekund, %d milisekund, %d mikrosekund\n\n", seconds, miliseconds, microseconds);
        fprintf(times, "Pelny czas: %d sekund, %d milisekund, %d mikrosekund\n", seconds, miliseconds, microseconds);
    }
    else{
        printf("Watek: %ld, czas: %d sekund, %d milisekund, %d mikrosekund\n", thread, seconds, miliseconds, microseconds);
        fprintf(times, "Watek: %d, czas: %d sekund, %d milisekund, %d mikrosekund\n", i, seconds, miliseconds, microseconds);
    }
}

void times_save(char *file_input, char* method, pthread_t * thread_array, struct timeval time_start){
    FILE* times = fopen(file_input, "a");
    if(times==NULL){
        printf("Nie udalo sie otworzyc pliku\n");
        exit(-1);
    }
    
    fprintf(times, "\nMetoda: %s, %d watkow\n", method, threads_number);
    for(int i=0; i<threads_number; i++){
        int *time;
        pthread_join(thread_array[i],(void **) &time);
        times_print(thread_array[i], *time, times, i + 1);
    }

    times_print(-1, get_time(time_start), times, -1);
    fclose(times);
}

int main(int argc, char **argv){
    if(argc!=5){
        printf("Niepoprawna ilosc argumentow\n");
        return -1;
    }

    threads_number = atoi(argv[1]);
    Div_method method;

    if(!strcmp("sign", argv[2]))
        method = SIGN;
    else if(!strcmp("block", argv[2]))
        method = BLOCK;
    else if(!strcmp("interleaved", argv[2]))
        method = INTERLEAVED;
    else{
        printf("Niepoprawny argument!\n");
        return -1;
    }

    char* file_input = argv[3];
    char* file_output = argv[4];

    image_load(file_input);

    histogram_array =(int **) calloc(threads_number, sizeof(int*));
    for(int i=0; i<threads_number; i++)
        histogram_array[i] =(int*)calloc(PIXEL_NUMBER, sizeof(int));
    
    struct timeval time_start;
    gettimeofday(&time_start, NULL);

    pthread_t * thread_array =(pthread_t*)calloc(threads_number, sizeof(pthread_t));
    int *index_array =(int*)calloc(threads_number, sizeof(int));

    for(int i=0; i<threads_number; i++){
        index_array[i] = i;
        if(method==SIGN)
            pthread_create(&thread_array[i], NULL, &sign, &index_array[i]);
        else if(method==BLOCK)
            pthread_create(&thread_array[i], NULL, &block, &index_array[i]);
        else 
            pthread_create(&thread_array[i], NULL, &interleaved, &index_array[i]);
    }

    times_save("pomiary.txt", argv[2], thread_array, time_start);
    to_file(file_output);
    for(int i=0; i<height; i++)
        free(img[i]);
    free(img);
    for(int i=0; i<threads_number; i++)
        free(histogram_array[i]);
    free(histogram_array);
    
    return 0;
}