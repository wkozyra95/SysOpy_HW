#define _GNU_SOURCE 99999
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>
#include <signal.h>
#include <pthread.h>
#include <stdbool.h>
#include <signal.h>
#include <limits.h>

int situation;
int sig;

void handler(int);
void* thread_func(void*);

void parse_input(int argc, char **pString);

int main(int argc, char* argv[]) {
    parse_input(argc, argv);

    pthread_t t;
    pthread_create(&t, NULL, thread_func, NULL);
    printf("PID: %d\n", getpid());

    if (situation == 2) {
        printf("Main thread %lu - block signal\n", pthread_self());
        sigset_t sigset;
        sigemptyset(&sigset);
        sigaddset(&sigset, sig);
        sigprocmask(SIG_SETMASK, &sigset, NULL);
    }

    if (situation == 3) {
        printf("Main thread %lu - handle signal\n", pthread_self());
        signal(sig, handler);
    }

    sleep(1);

    if(situation == 1 || situation == 2 || situation == 3) {
        printf("Main thread %lu - send signal to process\n", pthread_self());
        raise(sig);
    }

    if(situation == 4 || situation == 5) {
        printf("Main thread %lu- send signal to thread %d\n", pthread_self(), (int) t);
        pthread_kill(t, sig);
    }

    pthread_join(t,NULL);
    puts("Exit program");

    return 0;
}

void handler(int s){
    printf("Thread %li, PID %i signal handled %d\n", pthread_self(), getpid(), s);
}


void* thread_func(void* data) {
    if(situation == 3 || situation == 5){
        printf("new thread %lu - set handling signal\n",pthread_self());
        signal(sig,handler);
    }
    if(situation == 4) {
        printf("new thread%lu- set blocking signal\n", pthread_self());
        sigset_t sigset;
        sigemptyset(&sigset);
        sigaddset(&sigset, sig);
        pthread_sigmask(SIG_SETMASK, &sigset, NULL);
    }

    while(1)
        pause();
    return NULL;
}

void parse_input(int argc, char *argv[]) {
    if(argc != 3){
        printf("Wrong number of arguments\n");
        exit(1);
    }

    char* to_int_error;
    situation = (int) strtol(argv[1], &to_int_error, 10);
    if (errno == ERANGE || 0 != strcmp(to_int_error, "\0") || situation <= 0 || situation > 5) {
        printf("situation is number between 0 an 5\n");
        exit(1);
    }

    if(strcmp(argv[2], "SIGUSR1") == 0)
        sig = SIGUSR1;
    else if(strcmp(argv[2], "SIGTERM") == 0)
        sig = SIGTERM;
    else if(strcmp(argv[2], "SIGKILL") == 0)
        sig = SIGKILL;
    else if(strcmp(argv[2], "SIGSTOP") == 0)
        sig = SIGSTOP;
    else {
        printf("unknown signal\n");
        exit(1);
    }

}

