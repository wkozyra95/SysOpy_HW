#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include "generator.h"


int main(int argc, char const *argv[]) {
    if(argc != 4){
        printf("Wrong number of arguments\n");
        return 1;
    }

    char* to_int_error;
    long data_size = strtol(argv[2], &to_int_error , 10);
    if(errno == ERANGE || 0!=strcmp(to_int_error,"\0") || data_size<=0){
        printf("data_size must be an positive integer: %s\n", argv[2]);
        return 1;
    }

    long number_of_rows = strtol(argv[3], &to_int_error , 10);
    if(errno == ERANGE || 0!=strcmp(to_int_error,"\0") || data_size<=0){
        printf("number of rows must be an positive integer: %s\n", argv[3]);
        return 1;
    }

    if(number_of_rows > INT_MAX || data_size > INT_MAX){
        printf("too large numbers\n");
        return 1;
    }

    generate(argv[1], (int)data_size, (int)number_of_rows);

    return 0;
}

