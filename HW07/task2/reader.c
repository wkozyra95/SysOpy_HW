#include <time.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <string.h>
#include <limits.h>
#include <sys/time.h>
#include <unistd.h>


#define SHM_SIZE (4096)* sizeof(int)


typedef struct access{
    key_t memory_key;
    int shmid;
    sem_t *semid;
    int* memaddr;
    char* path;
    int all_data;
    int counter;
} access_t;

long value_s;

access_t mem_access;
int not_end = 1;
void normal_exit(int sig){
    not_end = 0;
    printf("podsumowanie(%d) wartość %ld znaleziono %d razy \n",
            getpid(), value_s, mem_access.counter);
}

void free_resources();

void semaphore_up();

void semaphore_down();

int main(int argc, char* argv[]) {
    if (argc != 3 && argc != 4) {
        printf("Wrong number of arguments\n");
        return -1;
    }

    if(argc == 4 && strcmp(argv[3], "-u") != 0) {
        printf("Wrong number of arguments\n");
        return -1;
    }

    if(argc == 4)
        mem_access.all_data = 0;
    else
        mem_access.all_data = 1;

    char* to_int_error;
    value_s = strtol(argv[2], &to_int_error, 10);
    if (errno == ERANGE || 0 != strcmp(to_int_error, "\0") || value_s < 0 || value_s > INT_MAX) {
        printf("value_s must be an positive integer: %s\n", argv[2]);
        return 1;
    }

    srand((unsigned int) time(NULL));
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

    mem_access.memaddr = mmap(0, SHM_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, mem_access.shmid, 0);
    if(mem_access.memaddr == MAP_FAILED){
        printf("mmap failed\n");
        return -1;
    }

    mem_access.semid = sem_open(mem_access.path, 0, 0666, 0);
    if(mem_access.semid == SEM_FAILED){
        printf("Error while creating semaphore\n");
        return -1;
    }
    atexit(free_resources);
    signal(SIGINT, normal_exit);

    int* buffer = (mem_access.memaddr);

    struct timespec t;
    printf("Start reader\n");
    while(not_end){
        semaphore_up();
        int counter = 0;
        for(int i = 0; i<4096; i++){
            if(buffer[i] == value_s)
                counter ++;
        }
        mem_access.counter += counter;

        struct timeval time_val;
        gettimeofday(&time_val, NULL);

        if(mem_access.all_data) {
            printf("(%d %ld.%ld) znalazłem %d liczb o wartości %ld\n",
                getpid(), time_val.tv_sec, time_val.tv_usec , counter, value_s);
        }
        semaphore_down();
        t.tv_nsec = 1000000 * 100;
        t.tv_sec = 0;
        nanosleep(&t, NULL);
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
