#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <sys/stat.h>
#include <limits.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Wrong number of arguments\n");
        return -1;
    }

    struct stat st;
    int file_exist = lstat(argv[1], &st);
    if( file_exist == 0 && (S_IFIFO & st.st_mode) == 0){
        printf("File with that name already exist\n");
        return -1;
    }

    if(file_exist != 0){
        if(mkfifo(argv[1], 0666) != 0) {
            printf("Unable to create FIFO\n");
            return -1;
        }
    }

    int file = open(argv[1], O_RDONLY);
    char input[4096];
    char timestr[200];

    while(1){
        if(read(file, input, 4096) == 0)
            return 0;

        time_t t = time(NULL);
        struct tm lt;
        localtime_r(&t, &lt);
        strftime(timestr, sizeof(timestr), "%c", &lt);

        printf("%s - %s\n", timestr, input);

    }


}

