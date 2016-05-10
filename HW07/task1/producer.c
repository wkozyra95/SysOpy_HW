#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

typedef struct access{
    key_t memory_key;
    int shmid;
    int semid;
    int* memaddr;
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
    if (argc != 3) {
        printf("Wrong number of arguments\n");
        return -1;
    }

    char* path = argv[1];

    char* to_int_error;
    long id_ftok = strtol(argv[2], &to_int_error, 10);
    if (errno == ERANGE || 0 != strcmp(to_int_error, "\0") || id_ftok <= 0 || id_ftok > INT_MAX) {
        printf("ftok_id must be an positive integer: %s\n", argv[2]);
        return 1;
    }

    mem_access.memory_key = ftok(path, (int)id_ftok);
    if(mem_access.memory_key == -1){
        printf("Error in ftok\n");
        return 1;
    }

    mem_access.shmid = shmget(mem_access.memory_key, (50 + 3) * sizeof(int) , 0666);
    if(mem_access.shmid == -1){
        printf("Error while creating shared memory\n");
        return -1;
    }

    mem_access.memaddr = shmat(mem_access.shmid, NULL, 0);
    if(mem_access.memaddr == (int *) -1) {
        printf("Error while mem_accessing shared memory\n");
        return -1;
    }
    atexit(free_resources);
    signal(SIGINT, normal_exit);

    mem_access.semid = mem_access.memaddr[0];
    int* buffer = mem_access.memaddr + 3;

    struct timespec t;

    srand((unsigned int) time(NULL));
    long counter = 0;
    while(not_end){
        semaphore_up();
        if(mem_access.memaddr[2]<50){
            int number = rand();
            int index = (mem_access.memaddr[1] + mem_access.memaddr[2]) % 50;
            buffer[index] = number;
            mem_access.memaddr[2] ++;
            counter ++;

            struct timeval time_val;
            gettimeofday(&time_val, NULL);
            printf("(%d %ld.%ld) Dodałem liczbę: %d na pozycję %d. Liczba zadań oczekujących: %d.\n",
                   getppid(), time_val.tv_sec, time_val.tv_usec, number, index, mem_access.memaddr[2]);
            t.tv_nsec = 1000000 * 200;
            t.tv_sec = 0;
            nanosleep(&t, NULL);
        }
        semaphore_down();
    }

}

void semaphore_down() {

    struct sembuf sops[1];
    sops[0].sem_num = 0;
    sops[0].sem_op = -1;
    sops[0].sem_flg = 0;
    if (semop(mem_access.semid, sops, 1) == -1) {
        printf("Semaphore error\n");
        exit(-1);
    }
}


void semaphore_up() {
    struct sembuf sops[2];

    sops[0].sem_num = 0;
    sops[0].sem_op = 0;
    sops[0].sem_flg = 0;

    sops[1].sem_num = 0;
    sops[1].sem_op = 1;
    sops[1].sem_flg = 0;

    if (semop(mem_access.semid, sops, 2) == -1) {
        printf("Semaphore error\n");
        exit(-1);
    }
}

void free_resources(){
    if(shmdt(mem_access.memaddr) == -1)
        printf("Unsuccessful memory detachment\n");
    printf("Exit producer\n");
}
