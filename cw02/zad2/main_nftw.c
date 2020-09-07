#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#define __USE_XOPEN_EXTENDED 1
#include <ftw.h>
#include <stdbool.h>

const char date_format[] = "%d.%m.%Y %H:%M:%S";
int atime = -1, mtime = -1;
int max_depth = 100000;
char atime_sign, mtime_sign;

void print(const char* path, const struct stat *stat){
    char typeofelement[64] = "niezdefiniowany";
    if (S_ISREG(stat -> st_mode)) strcpy(typeofelement, "file");
    else if (S_ISDIR(stat -> st_mode)) strcpy(typeofelement, "dir");
    else if (S_ISLNK(stat -> st_mode)) strcpy(typeofelement, "slink");
    else if (S_ISCHR(stat -> st_mode)) strcpy(typeofelement, "char dev");
    else if (S_ISBLK(stat -> st_mode)) strcpy(typeofelement, "block dev");
    else if (S_ISFIFO(stat -> st_mode)) strcpy(typeofelement, "fifo");
    else if (S_ISSOCK(stat -> st_mode)) strcpy(typeofelement, "socket");

    struct tm tm_modification;
    struct tm tm_access;
    localtime_r(&stat -> st_mtime, &tm_modification);
    localtime_r(&stat -> st_atime, &tm_access);
    char str_modification[255];
    char str_access[255];
    strftime(str_modification, 255, date_format, &tm_modification);
    strftime(str_access, 255, date_format, &tm_access);

    printf("%s | typ: %s, rozmiar: %ld, data ostatniej modyfikacji: %s, data ostatniego dostepu: %s, liczba dowiazan: %ld\n\n",
    path, typeofelement, stat -> st_size, str_modification, str_access, stat -> st_nlink);
}

bool timeOK(int set_time, char sign, time_t file_time){
    time_t now;
    struct tm *time_info;
    time(&now);
    time_info = localtime(&now);
    time_t current_date = mktime(time_info);

    int time_difference = difftime(current_date, file_time) / (24*60*60);

    if ((sign == '+' && time_difference > set_time) || (sign == '-' && time_difference < set_time) || (sign == '=' && time_difference == set_time)) 
        return true;

    return false;
}


bool meetsConditons(const struct stat* stat){
    if (atime >=0 && !timeOK(atime, atime_sign, stat -> st_atime)) return false;
    if (mtime >=0 && !timeOK(mtime, mtime_sign, stat -> st_mtime)) return false;
    return true;
}

static int find(const char *fpath, const struct stat *stat, int typeflag, struct FTW *ftwbuf){
    if (ftwbuf -> level > max_depth) return 0;
    if (meetsConditons(stat)){
        print(fpath, stat);
    }
    return 0;
}

int main(int argc, char** argv){
    if (argc < 2){
        printf("Brakuje argumentow!");
        return -1;
    }

    int i = 1;
    char* path = argv[i++];

    while(i < argc){
        if (!strcmp(argv[i], "-mtime")){
            if (mtime != -1){
                printf("Too many declarations of mtimt");
                exit(-1);
            }
            i++;
            if (argv[i][0] == '-' || argv[i][0] == '+') mtime_sign = argv[i][0];
            else mtime_sign = '=';
            mtime = abs(atoi(argv[i]));
        }
        else if(!strcmp(argv[i], "-atime")){
            if (atime != -1){
                printf("Too many declarations of atimt");
                exit(-1);
            }
            i++;
            if (argv[i][0] == '-' || argv[i][0] == '+') atime_sign = argv[i][0];
            else atime_sign = '=';
            atime = abs(atoi(argv[i]));
        }
        else if (!strcmp(argv[i], "-maxdepth")){
            i++;
            max_depth = atoi(argv[i]);
        }
        i ++;
    }

    nftw(path, find, 10, FTW_PHYS);

    return 0;

}