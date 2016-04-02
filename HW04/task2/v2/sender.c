#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <signal.h>

int counter = 0;
int expected;
int pid;
int confirm = 0;
int lock = 0;

void count(int);
void sum_up_count(int);


void set_up();

int main(int argc, char* argv []) {
    if (argc != 2) {
        printf("Wrong number of arguments\n");
        return -1;
    }

    char *to_int_error;
    long n_signals = strtol(argv[1], &to_int_error, 10);
    if (errno == ERANGE || 0 != strcmp(to_int_error, "\0") || n_signals <= 0 || n_signals > INT_MAX) {
        printf("Expected number: %s\n", argv[2]);
        return 1;
    }

    expected = (int) n_signals;

    set_up();


    sleep(1);
    printf("start sending %li signals\n", n_signals);
    for (int i = 0; i < n_signals; i++) {
        //printf("sender: send USR1 %d\n", i);
        lock = 1;
        kill(pid, SIGUSR1);
        while(lock){}
        //printf("s:pause()\n");
    }
    printf("sender: received confirmations=%d\n", counter);
    counter = 0;
    kill(pid, SIGUSR2);
    confirm = 1;

    while(1){}
    return 0;
}

void count(int sig) {
    lock = 0;
    counter++;
    //printf("sender: received USR1 %d\n", counter);
    if (confirm == 1) {
        //printf("sender: confirmation sent\n");
        kill(pid, SIGUSR1);
    }
}


void sum_up_count(int sig){
    printf("sender: signals sent=%d\n", expected);
    printf("sender: signals received=%d\n", counter);
    sleep(1);
    exit(0);
}

void set_up() {

    pid = fork();
    char *name[] = {"./task2_v2_catcher", NULL};
    if (pid == 0) {
        execv(name[0], name);
        printf("error in execv\n");
        exit(-1);
    }


    if(signal(SIGUSR1, count) == SIG_ERR)
        printf("can't catch SIGTSTP");
    if(signal(SIGUSR2, sum_up_count) == SIG_ERR)
        printf("can't catch SIGTSTP");
}
