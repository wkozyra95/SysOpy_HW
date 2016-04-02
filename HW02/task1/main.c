#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include <sys/resource.h>
#include "lib_test.h"
#include "sys_test.h"


int main(int argc, char const *argv[]) {
    if(argc != 4){
        printf("Wrong number of arguments\n");
        return 1;
    }

    char* to_int_error;
    long data_size = strtol(argv[2], &to_int_error , 10);
    if(errno == ERANGE || 0!=strcmp(to_int_error,"\0") || data_size <= 0 || data_size > INT_MAX){
        printf("data_size must be an positive integer: %s\n", argv[2]);
        return 1;
    }

    struct rusage rusage;
    clock_t my_clock;

    getrusage(RUSAGE_SELF, &rusage);
    my_clock = clock();
    double r_s = (double)my_clock / CLOCKS_PER_SEC;
    double u_s = (double)rusage.ru_utime.tv_sec + (double)rusage.ru_utime.tv_usec / (double)1e6;
    double s_s = (double)rusage.ru_stime.tv_sec + (double)rusage.ru_stime.tv_usec / (double)1e6;

    if(0 == strcmp("lib",argv[3])){
        lib_sort(argv[1], (int) data_size);
    } else if(0 == strcmp("sys",argv[3])){
        sys_sort(argv[1], (int) data_size);
    } else {
        printf("Unknown implementation: %s\n", argv[3]);
        return 1;
    }

    getrusage(RUSAGE_SELF, &rusage);
    my_clock = clock();
    double r_e = (double)my_clock / CLOCKS_PER_SEC;
    double u_e = (double)rusage.ru_utime.tv_sec + (double)rusage.ru_utime.tv_usec / (double)1e6;
    double s_e = (double)rusage.ru_stime.tv_sec + (double)rusage.ru_stime.tv_usec / (double)1e6;

    printf("real: %lf\n", r_e -  r_s);
    printf("user: %lf\n", u_e -  u_s);
    printf("sys: %lf\n", s_e -  s_s);
    return 0;
}

