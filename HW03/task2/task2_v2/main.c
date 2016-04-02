#include<getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "counter.h"

int matching_file(char* file_name);

int main(int argc, char* argv[]) {

    int vflag = 0;
    int wflag = 0;

    int c;
    opterr = 0;
    while ((c = getopt(argc, argv, "vw")) != -1) {
        switch (c) {
            case 'v':
                vflag = VERBOSE;
                break;
            case 'w':
                wflag = WAIT_TEST;
                break;
            default:
                printf("Wrong arguments\n");
                return -1;
        }
    }

    int result = fcount(vflag | wflag, argc, argv , matching_file);

    return result;
}



int matching_file(char* file_name) {
    char* env = getenv("EXT_TO_BROWSE");
    if(env == NULL || strlen(env) == 0) return 1;
    size_t patern_length = strlen(env);
    size_t path_length = strlen(file_name);

    return strcmp(env, &(file_name[path_length - patern_length])) == 0 &&
            file_name[path_length-patern_length-1] == '.';

}