#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <signal.h>
#include <unistd.h>
#include <sys/select.h>
#include <pthread.h>
#include "messages.h"


int max(int a, int b){ return a > b ? a : b; }
void check(bool condition, char* msg) {if(condition){printf("%s\n", msg);exit(-1);}}

typedef union {
    struct {
        struct in_addr ip;
        uint16_t port;
    };
    char socket_path[100];
} server_data_t;

typedef union {
    struct sockaddr_in addr_in;
    struct sockaddr_un addr_un;
} address_t;

struct {
    socket_type server_type;
    char username[MAX_USERNAME];
    address_t addr;
    server_data_t server;
    int socket_fd;
} C;

void parse_cmd(int argc, char **argv);
void setup_client();
void run_client();

int main(int argc, char* argv[]){
    printf("Parse cmd args\n");
    parse_cmd(argc, argv);

    printf("Setup client\n");
    setup_client();

    printf("Run client\n");
    run_client();

}

void* run_receiver(void* data);
void* run_sender(void* data);
void run_client() {
    pthread_t sender;
    pthread_create(&sender, NULL, run_sender, NULL);
    pthread_t receiver;
    pthread_create(&receiver, NULL, run_receiver, NULL);

    pthread_join(sender, NULL);
    pthread_join(receiver, NULL);
}

struct sockaddr* get_server_address(){
    if(C.server_type == UNIX)
        return (struct sockaddr *) &C.addr.addr_un;
    else
        return (struct sockaddr *) &C.addr.addr_in;
}

socklen_t get_server_address_size(){
    if(C.server_type == UNIX)
        return sizeof(struct sockaddr_un);
    else
        return sizeof(struct sockaddr_in);
}

void* run_sender(void* data){
    request_t new_request;
    strcpy(new_request.username, C.username);
    while(true){
        fgets(new_request.message_body, MAX_MESSAGE, stdin);
        check(
                send(C.socket_fd, &new_request, sizeof(request_t), 0) <= 0,
                "Unable to send"
        );
    }
}

void* run_receiver(void* data){
    request_t received_broadcast;

    while(true){
        fd_set fd_listener;
        FD_ZERO(&fd_listener);
        FD_SET(C.socket_fd, &fd_listener);

        check(
                select(C.socket_fd+1, &fd_listener, NULL, NULL, NULL) <= 0,
                "Error in select"
        );

        if(FD_ISSET(C.socket_fd, &fd_listener)) {
            ssize_t result = recv(C.socket_fd, &received_broadcast, sizeof(request_t), MSG_DONTWAIT);
            check(result < 0, "Unable to receive message");
            if (result > 0) {
                printf("Received broadcast\n user: %s\n message:%s",
                       received_broadcast.username, received_broadcast.message_body);
            } else {
                printf("Server disconnected\n");
                exit(0);
            }
        }


    }

}

void setup_cleanup();
void setup_connection_unix();
void setup_connection_inet() ;

void setup_client() {
    srand((unsigned int) time(NULL));
    setup_cleanup();
    if(C.server_type == UNIX){
        setup_connection_unix();
    } else if (C.server_type == INET) {
        setup_connection_inet();
    } else {
        printf("Wrong server type\n");
        exit(-1);
    }
}

void setup_connection_inet() {

    C.addr.addr_in.sin_family = AF_INET;
    C.addr.addr_in.sin_addr = C.server.ip;
    C.addr.addr_in.sin_port = htons(C.server.port);

    struct sockaddr_in client;
    client.sin_family = AF_INET;
    client.sin_port = htons(C.server.port  + (uint16_t)(rand()%100));
    client.sin_addr.s_addr = inet_addr("127.0.0.1");

    C.socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    check(C.socket_fd <= 0, "Illegal file decriptor");
    check(bind(C.socket_fd, (struct sockaddr *) &client, sizeof(struct sockaddr_in)) != 0,
          "bind unsuccessful\n");
    check(connect(C.socket_fd, (const struct sockaddr *) &C.addr.addr_in,
                  sizeof(struct sockaddr_in)) == -1, "Unable to connect");
}


void setup_connection_unix() {

    C.addr.addr_un.sun_family = AF_UNIX;
    strcpy(C.addr.addr_un.sun_path, C.server.socket_path);

    struct sockaddr_un client;
    client.sun_family = AF_UNIX;
    strcpy(client.sun_path, C.username);

    C.socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    check(C.socket_fd <= 0, "Illegal file decriptor");
    check(bind(C.socket_fd, (struct sockaddr *) &client, sizeof(struct sockaddr_un)) != 0,
          "bind unsuccessful\n");
    check(connect(C.socket_fd, (const struct sockaddr *) &C.addr.addr_un,
                  sizeof(struct sockaddr_un)) == -1, "Unable to connect");
}

void cleanup_client_connections(){
    //close(C.socket_fd);
    unlink(C.username);
}
void close_client(int sig) { exit(-1); }
void setup_cleanup() {
    signal(SIGINT, close_client);
    atexit(cleanup_client_connections);
}

void parse_cmd(int argc, char **argv) {
    if(argc < 3){
        printf("Wrong number of arguments\n");
        exit(-1);
    }

    if((argv[2][0] == 'A' && argc != 4) ||
       (argv[2][0] == 'B' && argc != 5) ||
       (argv[2][0] != 'A' && argv[2][0] != 'B')){
        printf("Wrong combination of arguments\n");
        exit(-1);
    }

    if(strlen(argv[1]) >= MAX_USERNAME){
        printf("Username too long\n");
        exit(-1);
    }
    strcpy(C.username, argv[1]);

    if(argv[2][0] == 'A' && strlen(argv[3]) < MAX_SOCKET_PATH){
        C.server_type = UNIX;
        strcpy(C.server.socket_path, argv[3]);
    } else if(argv[2][0] == 'B'){
        C.server_type = INET;
        check(inet_pton(AF_INET, argv[3], &C.server.ip) != 1, "Illegal ip");
        C.server.port = ((uint16_t) strtol(argv[4], NULL, 10));
    }
}