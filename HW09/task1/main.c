#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>

void* philosophize(void* id);

pthread_t philosophers[5];
sem_t forks[6];
sem_t waiter;

int main(int argc, char* argv[]){
    for(int i = 0; i<6; i++){
        sem_init(forks + i, 0, 1);
    }
    sem_init(&waiter, 0, 4);
    
    int indices[] = {0, 1, 2, 3, 4};
    for (int i = 0; i < 5; i++) {
        pthread_create(philosophers + i, NULL, philosophize, indices + i);
    }
    for (int i = 0; i < 5; i++) {
        pthread_join(philosophers[i], NULL);
    }

    for(int i = 0; i < 6 ; i++){
        sem_destroy(forks + i);
    }
    return 0;
}

void * philosophize(void *id) {
    int n = *((int *) id);
    struct timespec tm;
    tm.tv_nsec = 10000;

    while (1) {
        sem_wait(&waiter);
        sem_wait(forks + n);
        sem_wait(forks + ((n + 1) % 6));

        printf("%d philosopher\n", n);
        sleep(1);

        sem_post(forks + ((n + 1) % 6));
        sem_post(forks + n);
        sem_post(&waiter);
        nanosleep(&tm, NULL);
    }
    return NULL;
}