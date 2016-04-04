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
    char *name[] = {"./task2_v3_catcher", NULL};
    if (pid == 0) {
        execv(name[0], name);
        printf("error in execv\n");
        kill(getppid(), SIGRTMIN+1 );
        exit(-1);
    }
    expected = (int) n_signals;

    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGRTMIN);
    sigaddset(&set, SIGRTMIN+1);

    struct sigaction usr1;
    usr1.sa_handler = count;
    usr1.sa_flags = 0;
    usr1.sa_mask = set;

    struct sigaction usr2;
    usr2.sa_handler = sum_up_count;
    usr2.sa_flags = 0;
    usr2.sa_mask = set;

    if(sigaction(SIGRTMIN, &usr1, NULL) == -1)
        printf("can't catch SIGUSR1");
    if(sigaction(SIGRTMIN+1, &usr2, NULL) == -1)
        printf("can't catch SIGUSR2");

    sleep(1);
    printf("start sending %li signals\n", n_signals);
    union sigval val;
    for (int i = 0; i < n_signals; i++){
        if(sigqueue(pid, SIGRTMIN, val)!=0){
            printf("too much signals\n");
            kill(pid, SIGINT);
            exit(0);
        }
    }
    if(sigqueue(pid, SIGRTMIN+1, val)!=0){
        printf("too much signals\n");
        kill(pid, SIGINT);
        exit(0);
    }

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
