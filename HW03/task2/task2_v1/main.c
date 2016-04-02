#include <getopt.h>
#include <stdio.h>
#include "counter.h"

int main(int argc, char* argv[]) {

    int vflag = 0;
    int wflag = 0;

    int c;
    opterr = 0;
    while ((c = getopt(argc, argv, "vw")) != -1) {
        switch (c) {
            case 'v':
                vflag = VERBOSE;
                break;
            case 'w':
                wflag = WAIT_TEST;
                break;
            default:
                printf("Wrong arguments\n");
                return -1;
        }
    }

    int result = fcount(vflag | wflag, argc, argv);

    return result;
}