#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int counter = 0;

void count(int);
void sum_up_count(int);

int main(int argc, char* argv []){
    if(signal(SIGUSR1, count) == SIG_ERR)
        printf("can't catch SIGTSTP");
    if(signal(SIGUSR2, sum_up_count) == SIG_ERR)
        printf("can't catch SIGTSTP");
    printf("start catching\n");
    while(1){}
    return 0;
}


void count(int sig){
    counter++;
}

void sum_up_count(int sig){
    int pid = getppid();
    for(int i = 0; i<counter ;i++){
        kill(pid, SIGUSR1);
    }
    kill(pid, SIGUSR2);
    exit(0);
}
