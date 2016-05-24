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
    volatile int planes_on_carrier;
    volatile int planes_in_air;
    volatile int planes_waiting_for_landing;
    volatile int planes_waiting_for_start;
} thread_data;


int main(int argc, char* argv[]) {
    if (argc != 4) {
        printf("Wrong number of arguments\n");
        return -1;
    }

    char *to_int_error;
    long N = strtol(argv[1], &to_int_error, 10);
    if (errno == ERANGE || 0 != strcmp(to_int_error, "\0") || N <= 0 || N > INT_MAX) {
        printf("N must be an positive integer: %s\n", argv[2]);
        return 1;
    }

    long K = strtol(argv[2], &to_int_error, 10);
    if (errno == ERANGE || 0 != strcmp(to_int_error, "\0") || K <= 0 || K > INT_MAX) {
        printf("N must be an positive integer: %s\n", argv[2]);
        return 1;
    }

    long P = strtol(argv[3], &to_int_error, 10);
    if (errno == ERANGE || 0 != strcmp(to_int_error, "\0") || P <= 0 || P > INT_MAX) {
        printf("N must be an positive integer: %s\n", argv[2]);
        return 1;
    }
    srand((unsigned int) time(NULL));

    printf("Create carrier\n");

    carrier_capacity = (int) N;
    carrier_priority = (int) K;

    thread_data.planes_in_air = (int) P;
    thread_data.planes_on_carrier = 0;
    thread_data.planes_waiting_for_landing = 0;
    thread_data.planes_waiting_for_start = 0;

    thread_data.planes = malloc(sizeof(pthread_t) * P);
    int* indices = malloc(sizeof(int) * P);

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
    for (int i = 0; i < P; i++) {
        indices[i] = i;
        pthread_create(thread_data.planes + i, NULL, fly, indices + i);
    }
    for (int i = 0; i < P; i++) {
        pthread_join(thread_data.planes[i], NULL);
    }

    free(thread_data.planes);
    free(indices);
}

void start(int plane_id){

    pthread_mutex_lock(thread_data.landing_mutex);
    thread_data.planes_waiting_for_start ++;
    printf("Plane %i want to took off \t %d/%d/%d/%d\n", plane_id,thread_data.planes_on_carrier,
           thread_data.planes_in_air, thread_data.planes_waiting_for_start, thread_data.planes_waiting_for_landing);
    pthread_mutex_unlock(thread_data.landing_mutex);


    pthread_mutex_lock(thread_data.start_f_mutex);
    pthread_mutex_lock(thread_data.landing_mutex);
    while(!(thread_data.planes_on_carrier > carrier_priority ||     //start if priority
            thread_data.planes_waiting_for_landing == 0 ||          //or if landing request = 0
            thread_data.planes_on_carrier == carrier_capacity)){    //or if carrier full

        pthread_cond_wait(thread_data.waiting_for_start, thread_data.landing_mutex);

    }

    thread_data.planes_waiting_for_start --;
    thread_data.planes_in_air ++;
    thread_data.planes_on_carrier --;
    printf("Plane %i  took off \t\t %d/%d/%d/%d\n", plane_id,thread_data.planes_on_carrier,
           thread_data.planes_in_air, thread_data.planes_waiting_for_start, thread_data.planes_waiting_for_landing);

    pthread_cond_signal(thread_data.waiting_for_landing);
    pthread_mutex_unlock(thread_data.landing_mutex);
    pthread_mutex_unlock(thread_data.start_f_mutex);
}

void land(int plane_id){
    pthread_mutex_lock(thread_data.landing_mutex);
    thread_data.planes_waiting_for_landing ++;

    printf("Plane %i want to land \t\t %d/%d/%d/%d\n", plane_id,thread_data.planes_on_carrier,
           thread_data.planes_in_air, thread_data.planes_waiting_for_start, thread_data.planes_waiting_for_landing);
    pthread_mutex_unlock(thread_data.landing_mutex);


    pthread_mutex_lock(thread_data.land_f_mutex);
    pthread_mutex_lock(thread_data.landing_mutex);

    while((!(thread_data.planes_on_carrier <= carrier_priority ||
            thread_data.planes_waiting_for_start == 0)) || thread_data.planes_on_carrier == carrier_capacity){
        pthread_cond_wait(thread_data.waiting_for_landing, thread_data.landing_mutex);
    }

    thread_data.planes_waiting_for_landing --;
    thread_data.planes_in_air --;
    thread_data.planes_on_carrier ++;
    printf("Plane %i  landed \t\t %d/%d/%d/%d\n", plane_id,thread_data.planes_on_carrier,
           thread_data.planes_in_air, thread_data.planes_waiting_for_start, thread_data.planes_waiting_for_landing);

    pthread_cond_signal(thread_data.waiting_for_start);
    pthread_mutex_unlock(thread_data.landing_mutex);
    pthread_mutex_unlock(thread_data.land_f_mutex);
}

void* fly(void* id){
    int n_plane = *((int *) id);
    struct timespec tm;
    tm.tv_nsec = 1000000;
    tm.tv_sec = 0;
    printf("Start plane %i\n", n_plane);
    sleep(1);
    while(1) {
        land(n_plane);

        tm.tv_nsec = rand() % 1000;
        tm.tv_sec = 0;
        nanosleep(&tm, NULL);

        start(n_plane);
        tm.tv_nsec = rand() % 1000;
        tm.tv_sec = 0;
        nanosleep(&tm, NULL);
    }

}