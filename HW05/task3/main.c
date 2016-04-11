#include <stdio.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Wrong number of arguments\n");
        return -1;
    }

    FILE *pipein_fp, *pipeout_fp, * file_pipe;
    char readbuf[80];


    file_pipe = fopen(argv[1],"w");
    if(file_pipe == NULL){
        printf("Unable to create file\n");
        return -1;
    }
    dup2(fileno(file_pipe), STDOUT_FILENO);
    pipein_fp = popen("ls -l", "r");
    pipeout_fp = popen("grep ^d", "w");

    if(pipein_fp == NULL || pipeout_fp == NULL){
        printf("Unable to create pipe\n");
        return -1;
    }


    /* Processing loop */
    while(fgets(readbuf, 80, pipein_fp))
        fputs(readbuf, pipeout_fp);


    /* Close the pipes */

    pclose(pipein_fp);
    pclose(pipeout_fp);
    fclose(file_pipe);
}

