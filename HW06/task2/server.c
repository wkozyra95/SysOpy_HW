#include <stdio.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <mqueue.h>
#include <errno.h>

int n_clients = 0;
int queues[100];

struct message{
    long mtype;
    int id;
    int number;
    int result;
    char name[100];
};
typedef struct message message_t;


int register_client(message_t* data);
int ipc;
char* path;
void clean_up();
void exit_s(int sig){
    printf("\nExit\n");
    exit(0);
}

int main(int argc, char* argv[]){
    if(argc != 2){
        printf("Wrong number of arguments\n");
        return -1;
    }

    path = argv[1];
    message_t message_r;


    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(message_r);
    attr.mq_curmsgs = 0;

    ipc = mq_open(path, O_RDONLY|O_CREAT|O_EXCL, 0666, &attr);
    if(ipc == -1) {
        printf("Unable to use or create queue\n");
        return -1;
    }

    atexit(clean_up);
    signal(SIGINT, exit_s);
    srand((unsigned int) time(NULL));

    printf("Server started\n");

    while(1){
        mq_receive(ipc,  (char*)(&message_r), sizeof(message_r), NULL);
        // mtype = 1 client data
        if(message_r.mtype == 1){
            //printf("Recived register request from new client - queue: %s\n", message_r.name);
            int id = register_client(&message_r);
            message_r.mtype = 1; //server send clientid to client
            message_r.id = id;
            if(mq_send(queues[id], (char*)(&message_r), sizeof(message_r),0) == -1){
                printf("Unable to send response(klient:%d) errno: %d\n", id, errno);
                return -1;
            }
            message_r.mtype = 0;
        }

        // mtype = 2 client ready for task
        if(message_r.mtype == 2){
            //printf("Client %d is ready for new task\n", message_r.id);
            int id = message_r.id;
            message_r.mtype = 2;//server send new task to client
            message_r.number = rand();
            if(mq_send(queues[id], (char*)(&message_r), sizeof(message_r),0) == -1){
                printf("Unable to send response\n");
                return -1;
            }
            message_r.mtype = 0;
        }

        // mtype = 3 client task result
        if(message_r.mtype == 3){
            //printf("Client %d returned result %d\n", message_r.id, message_r.result);
            int id = message_r.id;
            int number = message_r.number;
            int isPrime = message_r.result;
            if(isPrime)
                printf("Liczba pierwsza: %d (klient: %d)\n",number, id);
            message_r.mtype = 0;
        }

        if(message_r.mtype == 4){
            mq_close(queues[message_r.id]);
        }

    }
    return 0;
}

int register_client(message_t* data) {

    char* mqueue = data->name;
    int client_new = mq_open(mqueue, O_WRONLY);
    if(client_new == -1) {
        printf("Unable to open queue %s\n", path);
        return -1;
    }
    queues[n_clients++] = client_new;
    return n_clients-1;
}


void clean_up(){
    for(int i = 0 ; i< n_clients;i++)
        mq_close(queues[i]);

    mq_close(ipc);
    mq_unlink(path);
}