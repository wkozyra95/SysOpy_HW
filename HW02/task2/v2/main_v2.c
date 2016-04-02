#define _XOPEN_SOURCE 500
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <linux/limits.h>
#include "driectory_tree.h"

int convert_permissions(const char *permissions);
char *convert_path_to_absolute(const char *path);

int main(int argc, char const *argv[]) {
    if(argc != 3){
        printf("Wrong number of arguments\n");
        return -1;
    }

    char* abs_path = convert_path_to_absolute(argv[1]);
    if(abs_path == NULL){
        printf("Incorrect path\n");
        return -1;
    }

    struct stat file_parameteres;
    int stat_result = stat(abs_path, &file_parameteres);
    if(stat_result<0 || S_ISDIR(file_parameteres.st_mode) == 0){
        printf("Not a directory\n");
        return -1;
    }

    const char *permissions = argv[2];
    size_t permission_length = strlen(argv[2]);

    if(permission_length != 9){
        printf("Incorrect permissions");
        return -1;
    }

    for(int i = 0; i<permission_length; i++){
        if(!((i%3 == 0 && permissions[i] == 'r') ||
           (i%3 == 1 && permissions[i] == 'w') ||
           (i%3 == 2 && permissions[i] == 'x') ||
           permissions[i] == '-')){
            printf("incorrect permissions\n");
            return -1;
        }
    }

    int perm = convert_permissions(permissions);

    if(0 != find_matching(abs_path, perm)){
        printf("Unable to read folder tree structure\n");
        return -1;
    }

    free(abs_path);
    return 0;

}

char* convert_path_to_absolute(const char* relative) {
    char* new_path = malloc(PATH_MAX);
    realpath(relative, new_path);
    return new_path;
}

int convert_permissions(const char *permissions) {
    int result = 0;
    if(permissions[0] == 'r') result |= S_IRUSR;
    if(permissions[1] == 'w') result |= S_IWUSR;
    if(permissions[2] == 'x') result |= S_IXUSR;
    if(permissions[3] == 'r') result |= S_IRGRP;
    if(permissions[4] == 'w') result |= S_IWGRP;
    if(permissions[5] == 'x') result |= S_IXGRP;
    if(permissions[6] == 'r') result |= S_IROTH;
    if(permissions[7] == 'w') result |= S_IWOTH;
    if(permissions[8] == 'x') result |= S_IXOTH;
    return result;
}