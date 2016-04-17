#include <stdio.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <signal.h>

struct client_data{
    long mtype;
    struct{
        int ipc;
    } content;
};
typedef struct client_data client_data;

struct client_id{
    long mtype;
    struct{
        int id;
    } content;
};
typedef struct client_id response_client_id;
typedef struct client_id recive_client_id;

struct server_task{
    long mtype;
    struct{
        int number;
    } content;
};
typedef struct server_task server_task;

struct task_result{
    long mtype;
    struct{
        int id;
        int number;
        int result;
    } content;
};

int isPrime(int number);

typedef struct task_result task_result;


void clean_up();
void exit_s(int sig){
    printf("Exit\n");
    exit(0);
}

int client;

int main(int argc, char* argv[]){
    if(argc != 3){
        printf("Wrong number of arguments\n");
        return -1;
    }

    char* path = argv[1];

    char* to_int_error;
    long id_ftok = strtol(argv[2], &to_int_error , 10);
    if(errno == ERANGE || 0!=strcmp(to_int_error,"\0") || id_ftok <= 0 || id_ftok > INT_MAX){
        printf("data_size must be an positive integer: %s\n", argv[2]);
        return 1;
    }


    key_t key;
    if ((key = ftok(path, (int)id_ftok)) == -1) {
        printf("Error in ftok\n");
        return -1;
    }

    int server = msgget(key, 0666);
    if(server == -1) {
        printf("Unable to use server queue\n");
        return -1;
    }

    client = msgget(IPC_PRIVATE, 0666);
    if(client == -1) {
        printf("Unable to create queue\n");
        return -1;
    }

    atexit(clean_up);
    signal(SIGINT, exit_s);

    int client_id;

    client_data client_data;
    response_client_id client_id_response;
    recive_client_id client_ready;
    server_task task;
    task_result result;

    client_data.mtype = 1;
    client_data.content.ipc = client;
    if(msgsnd(server, &client_data, sizeof(client_data.content), 0) == -1){
        printf("Error while registering in server\n");
        return -1;
    }

    printf("Client wait for response with id\n");
    if(msgrcv(client, &client_id_response, sizeof(client_id_response.content), 1,0)==-1){
        printf("Unable to acquire id\n");
        return -1;
    }
    client_id = client_id_response.content.id;
    printf("Client_id: %d\n", client_id);

    client_ready.mtype = 2;
    client_ready.content.id = client_id;
    result.mtype = 3;
    result.content.id = client_id;
    while(1) {
        if(msgsnd(server, &client_ready, sizeof(client_ready.content), 0) == -1){
            printf("Error sending\n");
            return -1;
        }

        if(msgrcv(client, &task, sizeof(client_id_response.content), 2, 0) == -1){
            printf("Error receiving\n");
            return -1;
        }
        result.content.number = task.content.number;
        result.content.result = isPrime(task.content.number);
        if(msgsnd(server, &result, sizeof(result.content), 0) == -1) {
            printf("Error\n");
            return -1;
        }

    }
    return 0;
}

int isPrime(int number) {
    for(int i = 2; i < number/2; i++ ){
        if(number % i == 0) return 0;
    }
    return 1;
}

void clean_up(){
    msgctl(client, IPC_RMID, NULL);
}