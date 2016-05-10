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

typedef struct access{
    key_t memory_key;
    int shmid;
    int semid;
    int* memaddr;
} access_t;

access_t mem_access;

void normal_exit(int sig){
    exit(0);
}

void free_resources();

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

    mem_access.shmid = shmget(mem_access.memory_key, (50 + 3) * sizeof(int) , 0666 | IPC_CREAT | IPC_EXCL);
    if(mem_access.shmid == -1){
        printf("Error while creating shared memory\n");
        return -1;
    }

    mem_access.semid = semget(IPC_PRIVATE, 1, 0666);
    if(mem_access.semid == -1){
        printf("Error while creating semaphore\n");
        return -1;
    }

    atexit(free_resources);
    signal(SIGINT, normal_exit);

    mem_access.memaddr = shmat(mem_access.shmid, NULL, 0);
    if(mem_access.memaddr == (int *) -1) {
        printf("Error while mem_accessing shared memory\n");
        return -1;
    }

    mem_access.memaddr[0] = mem_access.semid;

    while(1){
        pause();
    }

}

void free_resources(){
    if(semctl(mem_access.semid, 0, IPC_RMID) == -1)
        printf("Unable to remove semaphore\n");
    if(shmdt(mem_access.memaddr) == -1)
        printf("Unsuccessful memory detachment\n");
    if(shmctl(mem_access.shmid, IPC_RMID, NULL) == -1)
        printf("Unsuccessful memory release\n");
}