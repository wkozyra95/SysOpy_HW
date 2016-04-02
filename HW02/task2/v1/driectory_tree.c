#define _XOPEN_SOURCE 500
#include <dirent.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "driectory_tree.h"

#define MAX_PERMISSIONS (S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH)


int find_matching(const char *path, int permissions, int p_offset) {

    DIR* dir = opendir(path);
    if(dir == NULL){
        return -1;
    }
    char* new_path = malloc(sizeof(char)* (strlen(path) + 255));
    struct dirent* pDir = readdir(dir);
    while(pDir != NULL){
        if(strcmp(pDir->d_name, ".")==0 ||strcmp(pDir->d_name, "..")==0) {
            pDir = readdir(dir);
            continue;
        }

        strcpy(new_path, path);
        if(new_path[strlen(new_path) - 1] != '/') strcat(new_path, "/");
        strcat(new_path, pDir->d_name);

        struct stat st;
        int status = lstat(new_path, &st);

        if(status>=0 && S_ISDIR(st.st_mode) == 1){
            find_matching(new_path, permissions, p_offset);
            pDir = readdir(dir);
            continue;
        }

        if(status>=0 && (st.st_mode & MAX_PERMISSIONS) == permissions && S_ISREG(st.st_mode)){

            time_t t = st.st_mtime;
            struct tm lt;
            localtime_r(&t, &lt);
            char timestr[80];
            strftime(timestr, sizeof(timestr), "%c", &lt);


            printf("path: %s\nsize: %li\nlast modification: %s\n\n",
                   new_path + p_offset, st.st_size, timestr);

        }


        pDir = readdir(dir);
    }
    closedir(dir);
    free(new_path);
    return 0;
}