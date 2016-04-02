#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include "sys_test.h"

#define SC sizeof(char)

void sys_sort(const char *path, int row_size) {
    int handle = open (path, O_RDWR, NULL);
    if(handle  == -1){
        printf("Unable to open/change file\n");
        return;
    }
    struct stat file_info;
    stat(path, &file_info);
    long file_size = file_info.st_size;
    if((file_size-1) % row_size != 0){
        printf("incorrect row size\n");
        return;
    }

    char* current = malloc(SC*row_size);
    char* replacement = malloc(SC*row_size);

    lseek(handle, SC * row_size, 0);


    for(int i = 1; i < (file_size - 1)/row_size; i++){
        lseek(handle, SC * row_size * i, 0);
        ssize_t if_end = read(handle, current, SC * row_size );
        if(if_end != row_size ) {
            printf("error while reading file\n");
            return;
        };

        for(int j = i-1; j>=0; j--) {
            lseek(handle, SC * row_size * j, 0);
            read(handle, replacement, SC * row_size);
            if(current[0] < replacement[0]){
                write(handle, replacement, SC*row_size);
                lseek(handle, -SC * row_size * 2, 1);
                write(handle, current, SC * row_size);
            } else break;
        }

    }


    close(handle);
    free(current);
    free(replacement);
}
