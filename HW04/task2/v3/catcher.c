#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int counter = 0;

void count(int);
void sum_up_count(int);

int main(int argc, char* argv []){
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

    printf("start catching\n");
    while(1){}
    return 0;
}


void count(int sig){
    counter++;
}

void sum_up_count(int sig){
    union sigval val;
    int pid = getppid();
    for(int i = 0; i<counter ;i++){
        if(sigqueue(pid, SIGRTMIN, val)!=0){
            printf("too much signals\n");
            exit(0);
        }
    }
    if(sigqueue(pid, SIGRTMIN+1, val)!=0){
        printf("too much signals\n");
        exit(0);
    }
    exit(0);
}
