#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <memory.h>
#include <dirent.h>
#include <linux/limits.h>
#include <zconf.h>
#include <wait.h>
#include "counter.h"

int fcount(int flags, int argc, char *argv[], int (*match_file)(char*)) {
    int sum = 0;
    int file_counter = 0;
    char* path = getenv("PATH_TO_BROWSE");
    if(path == NULL){
        path = ".";
    }

    DIR* dir = opendir(path);
    if(dir == NULL){
        return -1;
    }
    char* new_path = malloc(PATH_MAX);
    struct dirent* pDir = readdir(dir);
    while(pDir != NULL) {
        if (strcmp(pDir->d_name, ".") == 0 || strcmp(pDir->d_name, "..") == 0) {
            pDir = readdir(dir);
            continue;
        }

        strcpy(new_path, path);
        if(new_path[strlen(new_path) - 1] != '/') strcat(new_path, "/");
        strcat(new_path, pDir->d_name);

        struct stat st;
        int status = lstat(new_path, &st);

        if(status != -1 && S_ISREG(st.st_mode) == 1  && match_file(new_path)) {
            file_counter++;
        }

        if(status != -1 && S_ISDIR(st.st_mode) == 1){
            setenv("PATH_TO_BROWSE", new_path, 1);
            int pid = vfork();
            if(pid ==  0){
                execv(argv[0], argv);
                printf("error in exec\n");
                _exit(-1);
            }

            if((flags & WAIT_TEST) != 0)
                sleep(10);

            int return_status;
            waitpid(pid, &return_status, 0);
            if (WIFEXITED(return_status)) {
                sum += WEXITSTATUS(return_status);
            }
        }

        pDir = readdir(dir);
    }
    setenv("PATH_TO_BROWSE", path, 1);
    free(new_path);

    if((flags & VERBOSE) != 0){
        printf("path: %s\n", path);
        printf("files in directory: %d\n", file_counter);
        printf("files in directory tree: %d\n\n", sum+file_counter);
    }
    return sum+file_counter;
}

