#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <limits.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Wrong number of arguments\n");
        return -1;
    }

    struct stat st;
    if(lstat(argv[1], &st) != 0 || S_IFIFO & st.st_mode == 0){
        printf("FIFO doesn't exists\n");
        return -1;
    }


    int file = open(argv[1], O_WRONLY);
    char input[4096];
    char data[4096];
    char timestr[200];
    while(1){
        scanf("%s", input);

        time_t t = time(NULL);
        struct tm lt;
        localtime_r(&t, &lt);
        strftime(timestr, sizeof(timestr), "%c", &lt);

        sprintf(data, "PID: %d - %s - %s\n",  getpid(),timestr, input);
        write(file, data, strlen(data)+1);
    }



}

