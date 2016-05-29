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



typedef struct {
    connection_state state;
    socket_type type;
    int fd;
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

void handle_request(request_t request, int fd);

void remove_clients_with_timeout(fd_set *ptr);

void register_client(int fd, socket_type type);

void unregister_client(int fd);

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
        int select_result = select(maxfd+1, &sockets, NULL, NULL, &tm);
        printf("select : %d\n", errno);
        for(int fd = 0; fd<maxfd+1; fd++){
            if(!FD_ISSET(fd, &sockets)) continue;
            if(fd == C.socket_in.fd) {
                struct sockaddr_in addr;
                socklen_t addr_size = sizeof(struct sockaddr_in);
                printf("test_startine\n");
                int new_client_descriptor = accept(fd, (struct sockaddr *) &addr, &addr_size);
                printf("test_stop inet\n");
                check(new_client_descriptor <= 0 , "Illegal file descriptor inet");
                maxfd = max(maxfd, new_client_descriptor);
                FD_SET(new_client_descriptor, &sockets_set);
                register_client(new_client_descriptor, INET);
            } else if (fd == C.socket_un.fd) {
                struct sockaddr_un addr;
                socklen_t addr_size = sizeof(struct sockaddr_un);
                printf("test_start\n");
                int new_client_descriptor = accept(fd, (struct sockaddr *) &addr, &addr_size);
                printf("test_stop\n");
                check(new_client_descriptor <= 0 , "Illegal file descriptor unix");
                printf("%d\n",fd);
                maxfd = max(maxfd, new_client_descriptor);
                FD_SET(new_client_descriptor, &sockets_set);
                register_client(new_client_descriptor, UNIX);
            } else {
                request_t request;
                printf("test_startww\n");
                ssize_t result = recv(fd, &request, sizeof(request_t), MSG_DONTWAIT);

                printf("test_stop rec\n");
                if( result > 0){
                    handle_request(request, fd);
                } else if(result == 0){
                    FD_CLR(fd, &sockets_set);
                    unregister_client(fd);
                    break;
                }

            }

        }

        remove_clients_with_timeout(&sockets_set);
    }

}

void unregister_client(int fd) {
    for(int i = 0; i<MAX_CLIENTS; i++){
        if(C.clients[i].state == CONNECTED && C.clients[i].fd == fd){
            close(fd);
            C.clients[i].state = DISCONNECTED;
            printf("Client removed\n");
            return;
        }
    }
}


void remove_clients_with_timeout(fd_set *socket_set) {
    for(int i = 0; i<MAX_CLIENTS; i++) {
        client_t *client = C.clients + i;
        if (client->state == DISCONNECTED) continue;

        if (time(NULL) - client->timestamp > 10){
            client->state = DISCONNECTED;
            FD_CLR(client->fd, socket_set);
            close(client->fd);
            printf("Client %s disconnected\n", client->name);
        }
    }


}


void handle_request(request_t request, int fd) {
    for(int i = 0; i<MAX_CLIENTS; i++){
        client_t* client = C.clients + i;
        if(client->state == DISCONNECTED) continue;
        if(client->fd == fd){
            client->timestamp = time(NULL);
        } else {
            check(
                    send(client->fd, &request, sizeof(request_t), 0) <= 0,
                    "Unable to send broadcast"
            );
        }
    }

}

void register_client(int fd, socket_type type) {
    for(int i = 0; i<MAX_CLIENTS; i++) {
        client_t* client = C.clients + i;
        if(client->state == DISCONNECTED) {
            client->type = type;
            client->fd = fd;
            client->state = CONNECTED;
            client->timestamp = time(NULL);
            printf("New client registered %d\n", fd);
            break;
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

    C.socket_in.fd = socket(AF_INET, SOCK_STREAM, 0);
    check(C.socket_in.fd <= 0, "Illegal file descriptor");
    check(bind(C.socket_in.fd, (struct sockaddr *) &C.socket_in.addr, sizeof(struct sockaddr_in)) != 0,
          "bind unsuccessful\n");
    check(listen(C.socket_in.fd, SOMAXCONN) != 0, "Unable to listen");
}

void setup_connection_unix() {

    C.socket_un.addr.sun_family = AF_UNIX;

    C.socket_un.fd = socket(AF_UNIX, SOCK_STREAM, 0);
    check(C.socket_un.fd <= 0, "Illegal file descriptor");
    check(bind(C.socket_un.fd, (struct sockaddr *) &C.socket_un.addr, sizeof(struct sockaddr_un)) != 0,
          "bind unsuccessful\n");
    check(listen(C.socket_un.fd, SOMAXCONN) != 0, "Unable to listen");

}


void cleanup_client_connections(){
    for(int i = 0; i<MAX_CLIENTS; i++){
        if(C.clients[i].state == CONNECTED){
            close(C.clients[i].fd);
        }
    }
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