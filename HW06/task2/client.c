#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <mqueue.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

struct message{
    long mtype;
    int id;
    int number;
    int result;
    char name[100];
};
typedef struct message message_t;


int isPrime(int number);
void clean_up();
void exit_s(int sig){
    printf("Exit\n");
    exit(0);
}
int client_id;
int client, server;
char* client_path;

int main(int argc, char* argv[]){
    if(argc != 3){
        printf("Wrong number of arguments\n");
        return -1;
    }

    char* path = argv[1];
    client_path = argv[2];
    if(argv[1][0] != '/' && argv[2][0] != '/'){
        printf("Wrong argument\n");
        return -1;
    }


    server = mq_open(path, O_WRONLY);
    if(server == -1) {
        printf("Unable to open queue %s\n", path);
        return -1;
    }

    message_t message;

    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(message);
    attr.mq_curmsgs = 0;

    client = mq_open(argv[2], O_RDONLY| O_CREAT, 0666, &attr);
    if(client == -1) {
        printf("Unable to create queue %s\n", client_path);
        return -1;
    }

    atexit(clean_up);
    signal(SIGINT, exit_s);




    message.mtype = 1;
    strcpy(message.name, client_path);
    if(mq_send(server, (char*)(&message), sizeof(message), 0) == -1){
        printf("Error while registering in server\n");
        return -1;
    }

    printf("Client wait for response with id\n");
    if(mq_receive(client, (char*)(&message), sizeof(message), 0) == -1){
        printf("Unable to acquire id\n");
        return -1;
    }
    client_id = message.id;
    printf("Client_id: %d\n", client_id);


    while(1) {
        message.mtype = 2;
        message.id = client_id;
        if(mq_send(server, (char*)(&message), sizeof(message), 0) == -1){
            printf("Error sending\n");
            return -1;
        }

        if(mq_receive(client, (char*)(&message), sizeof(message), 0) == -1){
            printf("Error receiving\n");
            return -1;
        }
        if(message.mtype == 2){
            message.mtype = 3;
            message.id = client_id;
            message.result = isPrime(message.number);
            if(mq_send(server, (char*)(&message), sizeof(message), 0) == -1){
                printf("Error\n");
                return -1;
            }
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
    message_t message;
    message.mtype = 4;
    message.id = client_id;
    mq_send(server, (char*)(&message), sizeof(message), 0);

    mq_close(client);
    mq_close(server);
    mq_unlink(client_path);
}