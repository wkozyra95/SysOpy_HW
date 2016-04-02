#include <stdio.h>
#include <stdlib.h>
#include "lib_test.h"

#define SC (sizeof(char))


void lib_sort(const char *path, int row_size) {

    FILE* file = fopen (path, "r+b");
    if(file == NULL){
        printf("Unable to create/change file\n");
        return;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    if((file_size-1) % row_size != 0){
        printf("incorrect row size\n");
        return;
    }

    char* current = malloc(SC*row_size);
    char* replacement = malloc(SC*row_size);

    fseek(file, SC * row_size, SEEK_SET);


    for(int i = 1; i < (file_size - 1)/row_size; i++){

        fseek(file, SC * row_size * (i), SEEK_SET);
        size_t if_end = fread(current, SC, SC*row_size, file);
        if(if_end != row_size ) {
            printf("error while reading file\n");
            return;
        };

        for(int j = i-1; j>=0; j--) {
            fseek(file, SC * row_size * j, SEEK_SET);
            fread(replacement, SC, SC* row_size, file);
            if(current[0] < replacement[0]){
                fwrite(replacement, SC, SC*row_size, file);
                fseek(file, -SC * row_size * 2, SEEK_CUR);
                fwrite(current, SC, SC * row_size, file);
            } else break;
        }

    }


    fclose(file);
    free(current);
    free(replacement);
}


