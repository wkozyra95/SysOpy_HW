#include <fcntl.h>
#include <string.h>
#include <limits.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "file_lock.h"

static int file;
static struct flock lock;

void list() {

    int mypid = fork();
    if(mypid == 0) {
        lock.l_start = 0L;
        lock.l_type = F_WRLCK;
        while (lock.l_type != F_UNLCK) {
            lock.l_whence = 0;
            lock.l_len = 0;
            lock.l_type = F_WRLCK;

            fcntl(file, F_GETLK, &lock);

            if (lock.l_type != F_UNLCK) {
                printf("pid: %8d  lock_type: %6s  offset: %8li  length: %8li\n",
                       lock.l_pid, (lock.l_type == F_WRLCK) ? "write" : "read",
                       lock.l_start, lock.l_len);

                if (lock.l_len == 0)
                    break;

                lock.l_start += lock.l_len;
            }
        }
        exit(0);
    } else {
        sleep(1);
    }
}

void lock_read(int number) {
    if(number == INT_MIN){
        printf("Not a number or missing argument %d\n", number);
        return;
    }
    lock.l_len = 1;
    lock.l_pid = getpid();
    lock.l_whence = SEEK_SET;
    lock.l_type = F_RDLCK;
    lock.l_start = number;
    if(fcntl(file, F_SETLK, &lock) != -1)
        printf("locked to read\n");
    else
        printf("unable to acquire a lock\n");
}

void lock_write(int number) {
    if(number == INT_MIN){
        printf("Not a number or missing argument\n");
        return;
    }
    lock.l_len = 1;
    lock.l_pid = getpid();
    lock.l_whence = SEEK_SET;
    lock.l_type = F_WRLCK;
    lock.l_start = number;
    if(fcntl(file, F_SETLK, &lock) != -1)
        printf("locked to write\n");
    else
        printf("unable to acquire a lock\n");
}

void unlock(int number) {
    if(number == INT_MIN){
        printf("Not a number or missing argument\n");
        return;
    }
    lock.l_len = 1;
    lock.l_pid = getpid();
    lock.l_whence = SEEK_SET;
    lock.l_type = F_UNLCK;
    lock.l_start = number;
    if(fcntl(file, F_SETLK, &lock) != -1)
        printf("lock released\n");
    else
        printf("lock don't exist\n");
}

void get_sign(int number) {
    if(number == INT_MIN){
        printf("Not a number or missing argument\n");
        return;
    }

    lock.l_len = 1;
    lock.l_pid = getpid();
    lock.l_whence = SEEK_SET;
    lock.l_type = F_RDLCK;
    lock.l_start = number;
    fcntl(file, F_GETLK, &lock);
    if(lock.l_type != F_UNLCK) {
        printf("value is locked\n");
        return;
    }

    char sign[1];
    lseek(file, number, 0);
    if(read(file,sign,1) != 0){
        printf("%c\n", sign[0]);
    } else {
        printf("Unable to read character\n");
    }

}

void set_sign(int number, char *character) {
    if(number == INT_MIN){
        printf("Not a number or missing argument %d\n", number);
        return;
    }

    if(character == NULL || strlen(character) != 1){
        printf("It's not a single character %s\n", character);
        return;
    }

    lock.l_len = 1;
    lock.l_pid = getpid();
    lock.l_whence = SEEK_SET;
    lock.l_type = F_WRLCK;
    lock.l_start = number;
    fcntl(file, F_GETLK, &lock);
    if(lock.l_type != F_UNLCK) {
        printf("value is locked\n");
        return;
    }

    lseek(file, number, 0);
    if(write(file,character,1)){
        printf("value inserted\n");
    } else {
        printf("Unable to insert value\n");
    }
}

int open_file(const char *path) {

    if((file = open(path, O_RDWR)) == -1) {
        printf("Unable to open file\n");
        return -1;
    }
    lock.l_len = 1;
    lock.l_pid = getpid();
    lock.l_whence = SEEK_SET;

    return 0;
}

void close_file() {
    close(file);
}
