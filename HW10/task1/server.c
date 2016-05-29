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

#define MAX_CLIENTS 100

int max(int a, int b){ return a > b ? a : b; }
void check(bool condition, char* msg) {if(condition){printf("%s\n", msg);}}

typedef union {
    struct sockaddr_in addr_in;
    struct sockaddr_un addr_un;
} client_address_t;

typedef struct {
    connection_state state;
    socket_type type;
    client_address_t address;
    time_t timestamp;
    char name[MAX_USERNAME];
} client_t;


struct {
    struct {
        int fd;
        struct sockaddr_un addr;
    } socket_un;
    struct {
        int fd;
        struct sockaddr_in addr;
    } socket_in;
    client_t clients[MAX_CLIENTS];
} C;

void parse_cmd(int argc, char **argv);
void setup_server();

void run_server();

int main(int argc, char* argv[]) {
    printf("Parse cmd\n");
    parse_cmd(argc, argv);

    printf("Setup server\n");
    setup_server();

    printf("Run server\n");
    run_server();


}

void handle_request(request_t request, struct sockaddr *addr, socket_type type);

void remove_clients_with_timeout(fd_set *ptr);

void run_server() {
    fd_set sockets_set;
    FD_ZERO(&sockets_set);
    FD_SET(C.socket_un.fd, &sockets_set);
    FD_SET(C.socket_in.fd, &sockets_set);
    int maxfd = max(C.socket_un.fd, C.socket_in.fd);
    while(true){
        struct timeval tm;
        tm.tv_sec = 1;
        tm.tv_usec = 0;
        fd_set sockets = sockets_set;
        select(maxfd+1, &sockets, NULL, NULL, &tm);
        if(FD_ISSET(C.socket_in.fd, &sockets)){
            request_t request;
            struct sockaddr_in addr;
            socklen_t addr_size = sizeof(struct sockaddr_in);
            check(
                    recvfrom(C.socket_in.fd, &request, sizeof(request_t), MSG_DONTWAIT, (struct sockaddr *) &addr, &addr_size) <= 0,
                    "Unable to recv message"
            );
            handle_request(request, (struct sockaddr *) &addr, INET);
        }
        if(FD_ISSET(C.socket_un.fd, &sockets)){
            request_t request;
            struct sockaddr_un addr;
            socklen_t addr_size = sizeof(struct sockaddr_un);
            check(
                    recvfrom(C.socket_un.fd, &request, sizeof(request_t), MSG_DONTWAIT, (struct sockaddr *) &addr, &addr_size) <= 0,
                    "Unable to recv message"
            );

            handle_request(request, (struct sockaddr *) &addr, UNIX);
        }
        remove_clients_with_timeout(NULL);
    }

}

void remove_clients_with_timeout(fd_set *ptr) {
    for(int i = 0; i<MAX_CLIENTS; i++) {
        client_t *client = C.clients + i;
        if (client->state == DISCONNECTED) continue;

        if (time(NULL) - client->timestamp > 15){
            client->state = DISCONNECTED;
            printf("Client %s disconected\n", client->name);
        }
    }


}

int get_fd(client_t* client){ return client->type == UNIX ? C.socket_un.fd : C.socket_in.fd;}
struct sockaddr* get_addr(client_t* client){
    return client->type == UNIX
           ? (struct sockaddr*) &client->address.addr_un
           : (struct sockaddr*) &client->address.addr_in;}

socklen_t get_addr_size(client_t* client){
    return client->type == UNIX
           ? sizeof(struct sockaddr_un)
           : sizeof(struct sockaddr_in);}

void handle_request(request_t request, struct sockaddr *addr, socket_type type) {
    bool is_client_registered = false;
    for(int i = 0; i<MAX_CLIENTS; i++){
        client_t* client = C.clients + i;
        if(client->state == DISCONNECTED) continue;
        if(strcmp(client->name, request.username) == 0){
            is_client_registered = true;
            client->timestamp = time(NULL);
        } else {
            check(
                    sendto(get_fd(client), &request, sizeof(request_t), 0, get_addr(client), get_addr_size(client)) <= 0,
                    "Unable to send broadcast"
            );
        }
    }
    if(!is_client_registered){
        for(int i = 0; i<MAX_CLIENTS; i++) {
            client_t* client = C.clients + i;
            if(client->state == DISCONNECTED) {
                client->type = type;
                if(type == UNIX){
                    client->address.addr_un.sun_family = AF_UNIX;
                    strcpy(client->address.addr_un.sun_path, request.username);
                } else {
                    struct sockaddr_in* addr_in = (struct sockaddr_in *) addr;
                    client->address.addr_in.sin_family = AF_INET;
                    client->address.addr_in.sin_addr.s_addr = addr_in->sin_addr.s_addr;
                    client->address.addr_in.sin_port = addr_in->sin_port;
                }
                client->state = CONNECTED;
                client->timestamp = time(NULL);
                strcpy(client->name, request.username);
                printf("New client registered %s\n", request.username);
                break;
            }
        }
    }

}

void setup_cleanup();
void setup_connection_unix();
void setup_connection_inet();
void setup_server() {
    setup_cleanup();
    for(int i = 0; i<MAX_CLIENTS; i++){
        C.clients[i].state = DISCONNECTED;
    }
    setup_connection_unix();
    setup_connection_inet();
}

void setup_connection_inet() {
    C.socket_in.addr.sin_family = AF_INET;
    C.socket_in.addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    C.socket_in.fd = socket(AF_INET, SOCK_DGRAM, 0);
    check(C.socket_in.fd <= 0, "Illegal file decriptor");
    check(bind(C.socket_in.fd, (struct sockaddr *) &C.socket_in.addr, sizeof(struct sockaddr_in)) != 0,
          "bind unsuccessful\n");
}

void setup_connection_unix() {

    C.socket_un.addr.sun_family = AF_UNIX;

    C.socket_un.fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    check(C.socket_un.fd <= 0, "Illegal file decriptor");
    check(bind(C.socket_un.fd, (struct sockaddr *) &C.socket_un.addr, sizeof(struct sockaddr_un)) != 0,
          "bind unsuccessful\n");

}


void cleanup_client_connections(){
    close(C.socket_un.fd);
    close(C.socket_in.fd);
    unlink(C.socket_un.addr.sun_path);
}
void close_client(int sig) { exit(0); }
void setup_cleanup() {
    signal(SIGINT, close_client);
    atexit(cleanup_client_connections);
}

void parse_cmd(int argc, char **argv) {
    if(argc != 3){
        printf("Wrong number of arguments\n");
        exit(-1);
    }

    C.socket_in.addr.sin_port = htons((uint16_t) strtol(argv[1], NULL, 10));
    check(errno != 0 || C.socket_in.addr.sin_port < 0, "Incorrect port number");

    check(strlen(argv[2]) >= MAX_SOCKET_PATH, "Too long socket path");
    strcpy(C.socket_un.addr.sun_path, argv[2]);
}