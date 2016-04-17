#include <stdio.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <mqueue.h>

int n_clients = 0;
int queues[100];

struct message{
    long mtype;
    int id;
    int number;
    int result;
};
typedef struct message message_t;


int register_client(message_t* data);
int ipc;
char* path;
void clean_up();
void exit_s(int sig){
    printf("Exit\n");
    exit(0);
}

int main(int argc, char* argv[]){
    if(argc != 2){
        printf("Wrong number of arguments\n");
        return -1;
    }

    path = argv[1];



    ipc = mq_open(path, O_RDONLY|O_CREAT);
    if(ipc == -1) {
        printf("Unable to use or create queue\n");
        return -1;
    }

    atexit(clean_up);
    signal(SIGINT, exit_s);
    srand((unsigned int) time(NULL));



    printf("Server started\n");
    message_t message_r;
    while(mq_receive(ipc,  (char*)(&message_r), sizeof(message_r), NULL) != -1){

        // mtype = 1 client data
        if(message_r.mtype == 1){
            //printf("Recived register request from new client - key: %d\n", client_data.content.key);
            int id = register_client(&message_r);
            message_r.mtype = 1; //server send clientid to client
            message_r.id = id;
            if(mq_send(queues[id], (char*)(&message_r), sizeof(message_r),0) == -1){
                printf("Unable to send response\n");
                return -1;
            }
        }

        // mtype = 2 client ready for task
        if(message_r.mtype == 2){
            //printf("Client %d is ready for new task\n", client_id_recived.content.id);
            int id = message_r.id;
            message_r.mtype = 2;//server send new task to client
            message_r.number = rand();
            if(mq_send(queues[id], (char*)(&message_r), sizeof(message_r),0) == -1){
                printf("Unable to send response\n");
                return -1;
            }
        }

        // mtype = 3 client task result
        if(message_r.mtype == 3){
            int id = message_r.id;
            int number = message_r.number;
            int isPrime = message_r.result;
            if(isPrime)
                printf("Liczba pierwsza: %d (klient: %d)\n",number, id);
        }

    }
    return 0;
}

int register_client(message_t* data) {

    int ipc = data->id;
    queues[n_clients++] = ipc;
    return n_clients-1;
}


void clean_up(){
    mq_close(ipc);
    mq_unlink(path);
}