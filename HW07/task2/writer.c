#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <sys/time.h>

#define SHM_SIZE (4096)* sizeof(int)


typedef struct access{
    key_t memory_key;
    int shmid;
    sem_t *semid;
    int* memaddr;
    char* path;
} access_t;

access_t mem_access;
int not_end = 1;
void normal_exit(int sig){
    not_end = 0;
}

void free_resources();

void semaphore_up();

void semaphore_down();

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Wrong number of arguments\n");
        return -1;
    }

    mem_access.path = argv[1];
    if(mem_access.path[0] != '/'){
        printf("Wrong name\n");
        return -1;
    }


    mem_access.shmid = shm_open(mem_access.path, O_RDWR, 0666);
    if(mem_access.shmid == -1){
        printf("Error while creating shared memory\n");
        return -1;
    }
    mem_access.memaddr = mmap(0, SHM_SIZE, PROT_WRITE | PROT_READ, MAP_SHARED, mem_access.shmid, 0);
    if(mem_access.memaddr == MAP_FAILED){
        printf("mmap failed %d\n", mem_access.shmid);
        return -1;
    }

    mem_access.semid = sem_open(mem_access.path, 0, 0666, 0);
    if(mem_access.semid == SEM_FAILED){
        printf("Error while creating semaphore\n");
        return -1;
    }
    atexit(free_resources);
    signal(SIGINT, normal_exit);

    int* buffer = mem_access.memaddr;
    struct timespec t;
    printf("Start writer\n");
    while(not_end){
        int tasks = rand() % 100;
        int start_v = rand() % 1000000;
        for(int i = 0;i< tasks && not_end; i++){
            semaphore_up();

            int index = rand() % 4096;
            buffer[index] = start_v;
            start_v ++;

            struct timeval time_val;
            gettimeofday(&time_val, NULL);

            printf("(%d %ld.%ld) write number %d in index %d - pending tasks %d\n", getpid(),
                   time_val.tv_sec, time_val.tv_usec, start_v, index, tasks - i);
            semaphore_down();
            t.tv_nsec = 1000000 * 100;
            t.tv_sec = 0;
            nanosleep(&t, NULL);
        }
    }
    return 0;
}

void semaphore_up() {

    if (sem_wait(mem_access.semid) == -1) {
        printf("Semaphore error\n");
        exit(-1);
    }
}


void semaphore_down() {
    if (sem_post(mem_access.semid) == -1) {
        printf("Semaphore error\n");
        exit(-1);
    }
}

void free_resources() {
    if(munmap(mem_access.memaddr, SHM_SIZE) == -1)
        printf("Unsuccessful memory detachment\n");
    if(sem_close(mem_access.semid) == -1)
        printf("Unable to remove semaphore\n");
    printf("Exit reader\n");
}