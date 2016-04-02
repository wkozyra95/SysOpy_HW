#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include "file_lock.h"

int to_int(char* number);

int main(int argc, char const *argv[]) {
    if(argc != 2){
        printf("Wrong number of arguments\n");
        return -1;
    }

    struct stat file_parameteres;
    int stat_result = stat(argv[1], &file_parameteres);
    if(stat_result<0 || S_ISREG(file_parameteres.st_mode) == 0){
        printf("File does not exist\n");
        return -1;
    }

    if(open_file(argv[1]) != 0) return -1;

    char* ignored_signs = " .,-\t\n";

    int flush_tmp;
    char* command = malloc(1000);
    char* command_part = NULL;

    while(1){
        command_part = strtok(NULL, ignored_signs);
        while(command_part != NULL) command_part = strtok(NULL, ignored_signs);

        printf(">>> ");
        command = fgets(command, 1000, stdin);
        if(strlen(command) == 999) {
            while ((flush_tmp = getchar()) != '\n' && flush_tmp != EOF);
        }


        command_part = strtok(command, ignored_signs);

        if(command_part == NULL) continue;
        if(strcmp(command_part, "exit") == 0) break;
        else if(strcmp(command_part, "list") == 0){
            list();
        } else if(strcmp(command_part, "lock") == 0){
            char* lock_type = strtok(NULL, ignored_signs);
            if(lock_type == NULL) {
                printf("invalid command\n");
                continue;
            }

            if(strcmp(lock_type, "read") == 0){
                char* str_number = strtok(NULL, ignored_signs);
                lock_read(to_int(str_number));
            } else if(strcmp(lock_type, "write") == 0){
                char* str_number = strtok(NULL, ignored_signs);
                lock_write(to_int(str_number));
            } else {
                printf("invalid command\n");
            }
        } else if(strcmp(command_part, "unlock") == 0){
            char* str_number = strtok(NULL, ignored_signs);
            unlock(to_int(str_number));
        } else if(strcmp(command_part, "read") == 0){
            char* str_number = strtok(NULL, ignored_signs);
            get_sign(to_int(str_number));
        } else if(strcmp(command_part, "write") == 0){
            char* str_number = strtok(NULL, ignored_signs);
            char* character = strtok(NULL, ignored_signs);
            set_sign(to_int(str_number), character);
        } else {
            printf("invalid command\n");
        }
    }
    free(command);
    close_file();
}


int to_int(char *number) {
    if(number == NULL){
        return INT_MIN;
    }

    char* to_int_error;
    long converted_number = strtol(number, &to_int_error , 10);
    if(errno == ERANGE || 0!=strcmp(to_int_error,"\0") ||
            converted_number < 0 || converted_number > INT_MAX){
        printf("Not a number: %s\n", number);
        return -1;
    }

    return (int) converted_number;
}
