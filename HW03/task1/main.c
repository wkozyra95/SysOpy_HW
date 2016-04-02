#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include "test_fork.h"

int main(int argc, char* argv[]) {
    if (argc != 5) {
        printf("Wrong number of arguments\n");
        return -1;
    }

    int list_cycles[] = {0, 0, 0, 0};
    for (int i = 1; i < 5; i++) {
        char *to_int_error = NULL;
        long number_of_cycles = strtol(argv[i], &to_int_error, 10);
        if (errno == ERANGE || 0 != strcmp(to_int_error, "\0") ||
            number_of_cycles <= 0 || number_of_cycles > INT_MAX) {
            printf("number_of_cycles must be an positive integer: %s\n", argv[2]);
            return 1;
        }
        list_cycles[i - 1] = (int) number_of_cycles;
    }


    test_fork(list_cycles);
    test_vfork(list_cycles);
    test_clone(list_cycles);
    test_vclone(list_cycles);

}