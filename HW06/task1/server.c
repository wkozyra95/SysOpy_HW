#include <stdio.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <signal.h>

int n_clients = 0;
int queues[100];

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
typedef struct task_result task_result;

int register_client(client_data* queue);
int ipc;
void clean_up();
void exit_s(int sig){
    printf("Exit\n");
    exit(0);
}

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
    ipc = msgget(key, IPC_CREAT| IPC_EXCL | 0666);
    if(ipc == -1) {
        printf("Unable to use or create queue\n");
        return -1;
    }

    atexit(clean_up);
    signal(SIGINT, exit_s);
    srand((unsigned int) time(NULL));

    client_data client_data;
    response_client_id client_id_response;
    recive_client_id client_id_recived;
    server_task task;
    task_result result;

    printf("Server started\n");
    while(1){
        // mtype = 1 client data
        if(msgrcv(ipc, &client_data, sizeof(client_data.content), 1, IPC_NOWAIT)!=-1){
            //printf("Recived register request from new client - key: %d\n", client_data.content.key);
            int id = register_client(&client_data);
            client_id_response.mtype = 1; //server send clientid to client
            client_id_response.content.id = id;
            if(msgsnd(queues[id], &client_id_response, sizeof(client_id_response.content),0) == -1){
                printf("Unable to send resonse\n");
                return -1;
            }
        }

        // mtype = 2 client ready for task
        if(msgrcv(ipc, &client_id_recived, sizeof(client_id_recived.content), 2, IPC_NOWAIT)!=-1){
            //printf("Client %d is ready for new task\n", client_id_recived.content.id);
            int id = client_id_recived.content.id;
            task.mtype = 2;//server send new task to client
            task.content.number = rand();
            msgsnd(queues[id], &task, sizeof(task.content),0);
        }

        // mtype = 3 client task result
        if(msgrcv(ipc, &result, sizeof(result.content), 3, IPC_NOWAIT)!=-1){
            int id = result.content.id;
            int number = result.content.number;
            int isPrime = result.content.result;
            if(isPrime)
                printf("Liczba pierwsza: %d (klient: %d)\n",number, id);
        }

    }
    return 0;
}

int register_client(client_data* data) {

    int ipc = data->content.ipc;
    queues[n_clients++] = ipc;
    return n_clients-1;
}


void clean_up(){
    msgctl(ipc, IPC_RMID, NULL);
}