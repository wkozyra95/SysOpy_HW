#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <pthread.h>
#include <unistd.h>

#define MS 1000000

void* fly(void* id);

int carrier_capacity;
int carrier_priority;

struct data {
    pthread_t* planes;
    pthread_cond_t* waiting_for_landing;
    pthread_cond_t* waiting_for_start;
    pthread_mutex_t* landing_mutex;
    pthread_mutex_t* start_f_mutex;
    pthread_mutex_t* land_f_mutex;
    int planes_on_carrier;
    int planes_in_air;
    int planes_waiting_for_landing;
    int planes_waiting_for_start;
} thread_data;


int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Wrong number of arguments\n");
        return -1;
    }

    char *to_int_error;
    long N = strtol(argv[1], &to_int_error, 10);
    if (errno == ERANGE || 0 != strcmp(to_int_error, "\0") || N <= 0 || N > INT_MAX) {
        printf("N must be an positive integer: %s\n", argv[2]);
        return 1;
    }

    long K = strtol(argv[1], &to_int_error, 10);
    if (errno == ERANGE || 0 != strcmp(to_int_error, "\0") || K <= 0 || K > INT_MAX) {
        printf("N must be an positive integer: %s\n", argv[2]);
        return 1;
    }
    srand((unsigned int) time(NULL));

    printf("Create carrier\n");

    carrier_capacity = (int) N;
    carrier_priority = (int) K;

    thread_data.planes_in_air = 0;
    thread_data.planes_on_carrier = carrier_capacity;
    thread_data.planes_waiting_for_landing = 0;
    thread_data.planes_waiting_for_start = 0;

    thread_data.planes = malloc(sizeof(pthread_t) * carrier_capacity);
    int* indices = malloc(sizeof(int) * carrier_capacity);

    printf("Create landing control\n");
    pthread_cond_t landing_condition;
    pthread_cond_t takeoff_condition;
    thread_data.waiting_for_landing = &landing_condition;
    thread_data.waiting_for_start = &takeoff_condition;
    pthread_cond_init(thread_data.waiting_for_landing, NULL);
    pthread_cond_init(thread_data.waiting_for_start, NULL);

    pthread_mutex_t* all_mutex = malloc(sizeof(pthread_mutex_t) * 3);
    thread_data.landing_mutex = all_mutex;
    thread_data.start_f_mutex = all_mutex + 1;
    thread_data.land_f_mutex = all_mutex + 2;
    pthread_mutex_init(thread_data.landing_mutex, NULL);
    pthread_mutex_init(thread_data.start_f_mutex, NULL);
    pthread_mutex_init(thread_data.land_f_mutex, NULL);

    printf("Start planes\n");
    for (int i = 0; i < carrier_capacity; i++) {
        indices[i] = i;
        pthread_create(thread_data.planes + i, NULL, fly, indices + i);
    }
    for (int i = 0; i < carrier_capacity; i++) {
        pthread_join(thread_data.planes[i], NULL);
    }

    free(thread_data.planes);
    free(indices);
}

void start(int plane_id){
    pthread_mutex_lock(thread_data.landing_mutex);
    thread_data.planes_waiting_for_start ++;

    pthread_mutex_lock(thread_data.start_f_mutex);

    while(thread_data.planes_on_carrier > carrier_priority &&
                thread_data.planes_waiting_for_landing > 0){
        pthread_cond_wait(thread_data.waiting_for_start, thread_data.landing_mutex);
    }

    thread_data.planes_waiting_for_start --;
    thread_data.planes_in_air ++;
    thread_data.planes_on_carrier --;
    printf("Plane %lu took off \t\t %d/%d\n", pthread_self(),thread_data.planes_on_carrier, thread_data.planes_in_air);

    pthread_cond_signal(thread_data.waiting_for_landing);
    pthread_mutex_unlock(thread_data.start_f_mutex);
    pthread_mutex_unlock(thread_data.landing_mutex);
}

void land(int plane_id){
    pthread_mutex_lock(thread_data.landing_mutex);
    thread_data.planes_waiting_for_landing ++;

    pthread_mutex_lock(thread_data.land_f_mutex);

    while(thread_data.planes_on_carrier <= carrier_priority &&
          thread_data.planes_waiting_for_start > 0){
        pthread_cond_wait(thread_data.waiting_for_landing, thread_data.landing_mutex);
    }

    thread_data.planes_waiting_for_landing --;
    thread_data.planes_in_air --;
    thread_data.planes_on_carrier ++;
    printf("Plane %lu landed \t\t %d/%d\n", pthread_self(),thread_data.planes_on_carrier, thread_data.planes_in_air);

    pthread_cond_signal(thread_data.waiting_for_start);
    pthread_mutex_unlock(thread_data.land_f_mutex);
    pthread_mutex_unlock(thread_data.landing_mutex);
}

void* fly(void* id){
    int n_plane = *((int *) id);
    struct timespec tm;
    tm.tv_nsec = 0;
    tm.tv_sec = 1;
    printf("Start plane %lu\n", pthread_self());
    while(1) {
        //printf("prestart plane %lu\n", pthread_self());
        start(n_plane);
        //printf("afterstart plane %lu\n", pthread_self());

        tm.tv_sec = rand() % 5;
        nanosleep(&tm, NULL);

        land(n_plane);
        tm.tv_sec = rand() % 5;
        nanosleep(&tm, NULL);
    }

}