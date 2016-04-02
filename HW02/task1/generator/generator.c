#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include "generator.h"

void generate(const char *path, int size, int rows ) {
    srand(time(NULL));


    FILE * generated_file = fopen (path, "w+");
    if(generated_file == NULL){
        printf("Unable to create/change file\n");
        return;
    }

    for(int i = 0; i < rows; i++){
        char* row = malloc(sizeof(char)*size);
        for(int j = 0; j < size; j++){
            row[j] = (char) ((rand() % 10) + 48);
        }
        fwrite(row, sizeof(char), size*sizeof(char), generated_file);

        free(row);
    }

    fwrite("\xA", sizeof(char), sizeof(char), generated_file);
    fclose(generated_file);
}

