#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

int counter = 0;

void count(int);
void sum_up_count(int);

int main(int argc, char* argv []){
    struct sigaction usr1;
    usr1.sa_handler = count;
    usr1.sa_flags = 0;

    struct sigaction usr2;
    usr2.sa_handler = sum_up_count;
    usr2.sa_flags = 0;

    if(sigaction(SIGRTMIN, &usr1, NULL) == -1)
        printf("can't catch SIGTERM");
    if(sigaction(SIGRTMIN + 1, &usr2, NULL) == -1)
        printf("can't catch SIGTERM");
    printf("start catching\n");
    while(1){pause();}
    return 0;
}


void count(int sig){
    counter++;
}

void sum_up_count(int sig){
    int pid = getppid();
    for(int i = 0; i<counter ;i++){
        kill(pid, SIGRTMIN);
    }
    kill(pid, SIGRTMIN + 1);
    exit(0);
}
