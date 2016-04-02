#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

int counter = 0;
int pid;
int lock = 1;
volatile int confirm;
void count(int);
void sum_up_count(int);

void set_up();

int main(int argc, char* argv []){
    set_up();

    confirm = 1;
    printf("catcher: start catching\n");

    while(1){}

    return 0;
}




void count(int sig){
    //printf("catcher: received USR1 %d\n", counter);
    lock = 0;
    counter++;
    if(confirm == 1){
        //printf("catcher: send confirmation USR1 %d\n", counter);
        kill(pid, SIGUSR1);

    }
}

void sum_up_count(int sig){
    printf("catcher: received USR2\n");
    int local_counter = counter;
    confirm = 0;
    for(int i = 0; i<local_counter ;i++) {
        //printf("catcher: kill %d\n", i);
        lock = 1;
        kill(pid, SIGUSR1);
        while(lock){};
        //printf("c:pause()\n");
    }
    kill(pid, SIGUSR2);
    exit(0);
}

void set_up() {
    pid = getppid();
    if(signal(SIGUSR1, count) == SIG_ERR)
        printf("can't catch SIGTSTP");
    if(signal(SIGUSR2, sum_up_count) == SIG_ERR)
        printf("can't catch SIGTSTP");

}