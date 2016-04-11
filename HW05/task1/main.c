
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <wait.h>


int main(int argc, char* argv[]){
    if(argc != 2){
        printf("Wrong number of arguments\n");
        return -1;
    }

    char* pipe_1[] = {"tr" ,  "\'[:lower:]\'",  "\'[:upper:]\'", NULL };
    char* pipe_2[] = {"fold", "-w", argv[1], NULL };


    char* to_int_error;
    long max_in_line = strtol(argv[1], &to_int_error , 10);
    if(errno == ERANGE || 0!=strcmp(to_int_error,"\0") || max_in_line <= 0 || max_in_line > INT_MAX){
        printf("Expected number: %s\n", argv[2]);
        return 1;
    }


    int fd[2];
    pipe(fd);
    int pid= fork();
    if (pid == 0) {
        close(fd[1]);
        dup2(fd[0], STDIN_FILENO);
        execvp(pipe_2[0], pipe_2);
    } else {
        close(fd[0]);
        dup2(fd[1], STDOUT_FILENO);
        execvp(pipe_1[0], pipe_1);
    }

    close(fd[0]);
    close(fd[1]);

}