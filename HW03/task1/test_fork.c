#define _XOPEN_SOURCE 500
#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <wait.h>
#include <zconf.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <time.h>
#include <stdlib.h>
#include "test_fork.h"

struct process_time{
    double real;
    double user_process;
    double user_child;
    double sys_process;
    double sys_child;
};
typedef struct process_time process_time_t;

int counter = 0;
process_time_t time_val[4];

void stop_clock(int i);
void start_clock(int i);
void print_clocks(char* string, int *number_of_cycles);


void test_fork(int* list_tests) {
    for(int j =0; j< 4; j++) {
        int number_of_cycles = list_tests[j];
        counter = 0;
        int return_status = 0;
        start_clock(j);
        for (int i = 0; i < number_of_cycles; i++) {
            int pid = fork();

            if (pid == 0) {
                //for(long i =0 ;i< 20000000;i++);
                counter++;
                _exit(0);
            }

            waitpid(pid, &return_status, 0);
            if (!WIFEXITED(return_status)) {
                printf("process returned error\n");
            }
        }
        stop_clock(j);
        printf("counter: %d\n", counter);
    }
    print_clocks("fork", list_tests);
}




void test_vfork(int* list_tests) {
    for(int j =0; j< 4; j++) {
        int number_of_cycles = list_tests[j];
        counter = 0;
        int return_status = 0;
        start_clock(j);
        for (int i = 0; i < number_of_cycles; i++) {
            int pid = vfork();

            if (pid == 0) {
                //for(long i =0 ;i< 20000000;i++);
                counter++;
                _exit(0);
            }

            waitpid(pid, &return_status, 0);
            if (!WIFEXITED(return_status)) {
                printf("process returned error\n");
            }
        }
        stop_clock(j);
        printf("counter: %d\n", counter);
    }
    print_clocks("vfork", list_tests);
}

int fn_clone(void* pVoid){
    counter++;
    return 0;
}


void test_clone(int* list_tests) {
    for(int j =0; j< 4; j++) {
        int number_of_cycles = list_tests[j];
        counter = 0;
        int return_status = 0;

        start_clock(j);
        for (int i = 0; i < number_of_cycles; i++) {
            size_t stack_size = 100;
            void*stack = mmap(NULL, stack_size, PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE | MAP_GROWSDOWN, -1, 0);

            int pid = clone( fn_clone, stack + stack_size , SIGCHLD, NULL);

            waitpid(pid, &return_status, 0);
            if (!WIFEXITED(return_status)) {
                printf("process returned error\n");
            }
            munmap(stack, stack_size);
        }
        stop_clock(j);
        printf("counter: %d\n", counter);
    }
    print_clocks("clone", list_tests);
}


void test_vclone(int* list_tests) {
    for(int j =0; j< 4; j++) {
        int number_of_cycles = list_tests[j];
        counter = 0;
        int return_status = 0;

        start_clock(j);
        for (int i = 0; i < number_of_cycles; i++) {
            size_t stack_size = 100;
            void*stack = mmap(NULL, stack_size, PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE | MAP_GROWSDOWN, -1, 0);

            int pid = clone( fn_clone, stack + stack_size , SIGCHLD | CLONE_VM | CLONE_VFORK, NULL);

            waitpid(pid, &return_status, 0);
            if (!WIFEXITED(return_status)) {
                printf("process returned error\n");
            }
            munmap(stack, stack_size);
        }
        stop_clock(j);
        printf("counter: %d\n", counter);
    }
    print_clocks("vclone", list_tests);
}



void start_clock(int i) {


    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    time_val[i].sys_process = (double)usage.ru_stime.tv_sec + (double)usage.ru_stime.tv_usec/1e6;
    time_val[i].user_process = (double)usage.ru_utime.tv_sec + (double)usage.ru_utime.tv_usec/1e6;

    struct rusage usage_child;
    getrusage(RUSAGE_CHILDREN, &usage_child);
    time_val[i].sys_child = (double)usage_child.ru_stime.tv_sec + (double)usage_child.ru_stime.tv_usec/1e6;
    time_val[i].user_child = (double)usage_child.ru_utime.tv_sec + (double)usage_child.ru_utime.tv_usec/1e6;

    struct timespec real_time;
    clock_gettime(CLOCK_REALTIME, &real_time);
    time_val[i].real = (double)real_time.tv_sec + (double)real_time.tv_nsec/1e9;
}

void stop_clock(int i) {
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    time_val[i].sys_process -= (double)usage.ru_stime.tv_sec + (double)usage.ru_stime.tv_usec/1e6;
    time_val[i].user_process -= (double)usage.ru_utime.tv_sec + (double)usage.ru_utime.tv_usec/1e6;

    struct rusage usage_child;
    getrusage(RUSAGE_CHILDREN, &usage_child);
    time_val[i].user_child -= (double)usage_child.ru_stime.tv_sec + (double)usage_child.ru_stime.tv_usec/1e6;
    time_val[i].user_child -= (double)usage_child.ru_utime.tv_sec + (double)usage_child.ru_utime.tv_usec/1e6;

    struct timespec real_time;
    clock_gettime(CLOCK_REALTIME, &real_time);
    time_val[i].real -= (double)real_time.tv_sec + (double)real_time.tv_nsec/1e9;

    time_val[i].real = time_val[i].real < 0 ? -time_val[i].real : time_val[i].real;
    time_val[i].sys_process = time_val[i].sys_process < 0 ? -time_val[i].sys_process : time_val[i].sys_process;
    time_val[i].sys_child = time_val[i].sys_child < 0 ? -time_val[i].sys_child : time_val[i].sys_child;
    time_val[i].user_process = time_val[i].user_process < 0 ? -time_val[i].user_process : time_val[i].user_process;
    time_val[i].user_child = time_val[i].user_child < 0 ? -time_val[i].user_child : time_val[i].user_child;

}


void print_clocks(char* string, int *number_of_cycles) {
    printf("function: %s  time: process\n", string);
    printf("N\treal\tsys\tusr\tsys+usr\n");
    for(int i = 0; i<4; i++){
        printf("%d\t%lf\t%lf\t%lf\t%lf\n", number_of_cycles[i], (time_val[i].real), (time_val[i].sys_process),
               (time_val[i].user_process), (time_val[i].sys_process + time_val[i].user_process));
    }


    printf("function: %s  time: child\n", string);
    printf("N\treal\tsys\tusr\tsys+usr\n");
    for(int i = 0; i<4; i++){
        printf("%d\t%lf\t%lf\t%lf\t%lf\n", number_of_cycles[i], (time_val[i].real), (time_val[i].sys_child),
               (time_val[i].user_child), (time_val[i].sys_child + time_val[i].user_child));
    }

    printf("function: %s  time: process+child\n", string);
    printf("N\treal\tsys\tusr\tsys+usr\n");
    for(int i = 0; i<4; i++){
        printf("%d\t%lf\t%lf\t%lf\t%lf\n", number_of_cycles[i], (time_val[i].real),
               (time_val[i].sys_child + time_val[i].sys_process),
               (time_val[i].user_child + time_val[i].user_process),
               (time_val[i].sys_child + time_val[i].user_child + time_val[i].sys_process + time_val[i].user_process));
    }
}
