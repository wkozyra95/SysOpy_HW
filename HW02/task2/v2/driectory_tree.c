#define _XOPEN_SOURCE 500
#include <dirent.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <asm-generic/errno-base.h>
#include <ftw.h>
#include <unistd.h>
#include <bits/errno.h>
#include "driectory_tree.h"

#define MAX_PERMISSIONS (S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH)

static int perm;


int print_matching(const char *filepath, const struct stat *info,
                             const int typeflag, struct FTW *pathinfo)
{

    if((info->st_mode & MAX_PERMISSIONS) == perm && S_ISREG(info->st_mode)){

        time_t t = info->st_mtime;
        struct tm lt;
        localtime_r(&t, &lt);
        char timestr[80];
        strftime(timestr, sizeof(timestr), "%c", &lt);


        printf("path: %s\nsize: %li\nlast modification: %s\n\n",
               filepath, info->st_size, timestr);
        return 0;

    }

    return 0;
}



int find_matching(const char *path, int permissions) {

    perm = permissions;

    int result = nftw(path, print_matching, 50, FTW_PHYS );
    if(result != 0) return -1;

    return 0;
}