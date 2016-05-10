#include <sys/mman.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <signal.h>
#include <unistd.h>
#include <semaphore.h>

#define SHM_SIZE (4096)* sizeof(int)

typedef struct access{
    key_t memory_key;
    int shmid;
    sem_t* semid;
    int* memaddr;
    char *path;
} access_t;

access_t mem_access;

void normal_exit(int sig){
    printf("Delete shared memory\n");
    exit(0);
}

void free_resources();

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


    mem_access.shmid = shm_open(mem_access.path, O_EXCL | O_CREAT | O_RDWR, 0666);
    if(mem_access.shmid == -1){
        printf("Error while creating shared memory\n");
        return -1;
    }
    if(ftruncate(mem_access.shmid, SHM_SIZE) == -1){
        printf("Error while resizing %d er=%d\n", mem_access.shmid, errno);
        return -1;
    }
    mem_access.memaddr = mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, mem_access.shmid, 0);
    if(mem_access.memaddr == MAP_FAILED){
        printf("mmap failed\n");
        return -1;
    }

    mem_access.semid = sem_open(mem_access.path, O_CREAT | O_EXCL, 0666, 0);
    if(mem_access.semid == SEM_FAILED){
        printf("Error while creating semaphore\n");
        return -1;
    }

    if(sem_init(mem_access.semid, 1, 1)){
        printf("Error while creating semaphore\n");
        return -1;
    }

    atexit(free_resources);
    signal(SIGINT, normal_exit);


    while(1){
        pause();
    }

}

void free_resources(){
    if(sem_close(mem_access.semid) == -1)
        printf("Unable to remove semaphore\n");
    if(sem_unlink(mem_access.path))
        printf("Unable to unlink semaphore\n");
    if(munmap(mem_access.memaddr, SHM_SIZE) == -1)
        printf("Unsuccessful memory detachment\n");
    if(shm_unlink(mem_access.path) == -1)
        printf("Unsuccessful memory release\n");
}