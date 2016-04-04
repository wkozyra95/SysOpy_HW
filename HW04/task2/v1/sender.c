#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <signal.h>

int counter = 0;
int expected;

void count(int);
void sum_up_count(int);


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

    int pid = fork();
    char *name[] = {"./task2_v1_catcher", NULL};
    if (pid == 0) {
        execv(name[0], name);
        printf("error in execv\n");
        kill(getppid(), SIGUSR2);
	exit(-1);
    }
    expected = (int) n_signals;


    if(signal(SIGUSR1, count) == SIG_ERR)
        printf("can't catch SIGTSTP");
    if(signal(SIGUSR2, sum_up_count) == SIG_ERR)
        printf("can't catch SIGTSTP");

    sleep(1);
    printf("start sending %li signals\n", n_signals);
    for (int i = 0; i < n_signals; i++){
        kill(pid, SIGUSR1);
    }
    kill(pid, SIGUSR2);


    while(1){pause();}
    return 0;
}


void count(int sig){
    counter++;
}

void sum_up_count(int sig){
    printf("signals sent: %d\n", expected);
    printf("signals received: %d\n", counter);
    exit(0);
}
