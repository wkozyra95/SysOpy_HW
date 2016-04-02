#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include "driectory_tree.h"

int convert_permissions(const char *permissions);

int main(int argc, char const *argv[]) {
    if(argc != 3){
        printf("Wrong number of arguments\n");
        return -1;
    }

    struct stat file_parameteres;
    int stat_result = stat(argv[1], &file_parameteres);
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
    int path_offset = (int) strlen(argv[1]) +1 ;
    if(0 != find_matching(argv[1], perm, path_offset)){
        printf("Unable to read folder tree structure\n");
        return -1;
    }


    return 0;

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