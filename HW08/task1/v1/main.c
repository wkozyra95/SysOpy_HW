#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <pthread.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <stdbool.h>

typedef struct thread_data{
    int record_per_read;
    int fd;
    pthread_mutex_t* mutex;
    pthread_t* threads;
    char* thread_canceled;
    int n_threads;
    char* search_word;
} thread_data_t;



void* new_thread(void* data);

int find_match(char *record, char *search_word);

int main(int argc, char* argv[]) {
    if (argc != 5) {
        printf("Wrong number of arguments\n");
        return -1;
    }

    char* to_int_error;
    long n_threads = strtol(argv[1], &to_int_error, 10);
    if (errno == ERANGE || 0 != strcmp(to_int_error, "\0") || n_threads < 0 || n_threads > INT_MAX) {
        printf("n_threads must be an positive integer: %s\n", argv[2]);
        return 1;
    }

    char* file_name = argv[2];

    long records_per_read = strtol(argv[3], &to_int_error, 10);
    if (errno == ERANGE || 0 != strcmp(to_int_error, "\0") || records_per_read < 0 || records_per_read > INT_MAX) {
        printf("N_records must be an positive integer: %s\n", argv[3]);
        return 1;
    }

    char* search_word = argv[4];



    printf("Create threads\n");
    thread_data_t data;
    data.record_per_read = (int) records_per_read;
    data.search_word = search_word;

    data.n_threads = (int) n_threads;
    data.threads = malloc(sizeof(pthread_t) * n_threads);
    data.mutex = malloc(sizeof(pthread_mutex_t));

    printf("Create mutex\n");
    pthread_mutex_init(data.mutex, NULL);
    printf("Open file\n");
    data.fd = open(file_name, O_RDONLY);

    printf("Start threads\n");
    for(int i = 0; i < n_threads; i++){
        pthread_create(data.threads + i, NULL, new_thread, &data);
    }

    for(int i = 0; i < n_threads; i++){
        if(pthread_join(data.threads[i], NULL) != 0){
            printf("Error while joining\n");
        }
    }
    printf("Clean up\n");
    close(data.fd);
    pthread_mutex_destroy(data.mutex);
    free(data.threads);
    free(data.mutex);
    printf("Exit\n");
    return 0;
}

typedef struct array{
    char** record;
    int size;
    pthread_mutex_t* mutex;
} array_t;

void free_array(array_t* data){

    for (int i = 0; i < data->size; i++) {
        free(data->record[i]);
    }
    free(data->record);
}

void* new_thread(void* data) {
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    sleep(1);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    thread_data_t *T = data;
    char **record_list = malloc(sizeof(char *) * T->record_per_read);

    array_t A;
    A.size = T->record_per_read;
    A.record = record_list;
    A.mutex = T->mutex;
    pthread_cleanup_push(free_array, &A);
            for (int i = 0; i < T->record_per_read; i++) {
                record_list[i] = malloc(1030);
            }
            while (1) {

                int records = 0;
                pthread_mutex_lock(T->mutex);
                for (int i = 0; i < T->record_per_read && read(T->fd, record_list[i], 1025) != 0; i++) {
                    record_list[i][1024] = 0;
                    records++;
                }
                pthread_mutex_unlock(T->mutex);
                if(records == 0) pthread_exit(0);
                pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
                pthread_testcancel();
                pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

                for (int i = 0; i < records; i++) {
                    char *text;
                    long id = strtol(record_list[i], &text, 10);
                    text += 1;
                    if (find_match(text, T->search_word)) {
                        printf("TID: %li RECORD_ID: %li\n", pthread_self(), id);
                        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
                        pthread_testcancel();
                        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
                        pthread_mutex_lock(T->mutex);
                        for (int j = 0; j < T->n_threads; j++) {
                            if (!pthread_equal(T->threads[j],pthread_self())) {
                                pthread_cancel(T->threads[j]);
                            }
                        }
                        pthread_mutex_unlock(T->mutex);
                        pthread_exit(0);
                    }
                }

            }
    pthread_cleanup_pop(0);
    return NULL;
}

int find_match(char *record, char *search_word) {
    int record_len = (int) strlen(record);
    int search_len = (int) strlen(search_word);


    for(int i = 0; i < record_len - search_len + 1; i++){
        int j = i;
        int equal = 1;
        for(int k = 0; k < search_len; k++){
            if(record[j] != search_word[k]){
                equal = 0;
                break;
            }
            j++;
        }
        if(equal){
            return 1;
        }
    }
    return 0;
}
