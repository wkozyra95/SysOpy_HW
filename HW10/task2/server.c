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
#include <time.h>

#include "massages.h"
#include "../task1/massages.h"

#define N_CLIENTS 100
int max(int a, int b){ return a>b?a:b; }
void check(bool condition, char* msg) {if(condition){printf("%s\n", msg);}}

typedef struct  {
    state_t client_state;
    time_t timestamp;
    int fd;
} client_t;

struct {
    uint16_t port;
    char* socket_path;
    fd_set descriptors;
    int unix_socket;
    int inet_socket;
    int max_fd;
    client_t clients[N_CLIENTS];
} C;

void setup_program();
void parse_arguments(int argc, char **argv);
void setup_communication() ;
void communicate();

int main(int argc, char* argv[]) {
    setup_program();
    parse_arguments(argc, argv);

    setup_communication();
    communicate();

    return 0;
}

void handle_message(struct sockaddr_un fd, int i);
void register_client(struct sockaddr *server_socket, socket_type param);
void delete_unactive_clients();

void communicate() {
    while(true) {
        printf("Wait for select\n");
        check(select(C.max_fd+1, &C.descriptors, NULL, NULL, NULL) > 0, "Select: new messages");
        printf("Select----------------------------\n");
        for(int fd = 0; fd<C.max_fd; fd++){
            if(!FD_ISSET(fd, &C.descriptors)) continue;

            if (fd == C.inet_socket){
                register_client(C.inet_socket, (NO_MESSAGE));
            } else if (fd == C.unix_socket){
                register_client(C.unix_socket, (NO_MESSAGE));
            } else {
                handle_message(fd, 0);
            }

        }
        delete_unactive_clients();
    }
}



void handle_message(struct sockaddr_un fd, int i) {
    request_t request;
    if(recv(fd, &request, sizeof(request), MSG_DONTWAIT) == -1) return;
    check(true, request.massage);
    int i;
    for (i = 0; i < N_CLIENTS; i++) {
        if (C.clients[i].client_state == DISCONNECTED) continue;

        if (C.clients[i].fd == fd) {
            C.clients[i].timestamp = time(NULL);
        } else {
            check(send(fd, &request, sizeof(request), 0) != -1, "Unable to send broadcast");
        }
    }

}

void delete_unactive_clients() {
    for (int i = 0; i< N_CLIENTS; i++){
        if(C.clients[i].client_state == CONNECTED && time(NULL) - C.clients[i].timestamp > 30){
            C.clients[i].client_state = DISCONNECTED;
            close(C.clients[i].fd);
        }
    }
}

void register_client(struct sockaddr *server_socket, socket_type param) {
    int client_fd;
    if((client_fd = accept(server_socket, NULL, 0)) == -1 ) return;
    int i;
    for(i = 0; i<N_CLIENTS; i++){
        if(C.clients[i].client_state == DISCONNECTED) break;
    }
    if(i == N_CLIENTS) return;


    C.clients[i].client_state = CONNECTED;
    C.clients[i].fd = client_fd;
    C.clients[i].timestamp = time(NULL);
    C.max_fd = max(C.max_fd, client_fd);
    FD_SET(client_fd, &C.descriptors);
}


int create_inet_socket();
int create_unix_socket();

void setup_communication() {
    for(int i =0 ;i<N_CLIENTS; i++){
        C.clients[i].client_state = DISCONNECTED;
    }
    FD_ZERO(&C.descriptors);
    C.inet_socket = create_inet_socket();
    C.unix_socket = create_unix_socket();
    FD_SET(C.inet_socket, &C.descriptors);
    FD_SET(C.unix_socket, &C.descriptors);
    C.max_fd = max(C.unix_socket, C.inet_socket);
    printf("Server ready\n");
}

int create_unix_socket() {
    unlink(C.socket_path);
    int socket_fd = socket(AF_UNIX, SOCK_DGRAM | SOCK_NONBLOCK, 0);
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, C.socket_path);

    check(bind(socket_fd, (const struct sockaddr *) &addr, sizeof(addr)) != 0, "Error: unix socket bind");
    //check(listen(socket_fd, SOMAXCONN) !=0, "Error: unix socket listen");
    return socket_fd;
}

int create_inet_socket() {
    int socket_fd = socket(AF_INET, SOCK_DGRAM , 0);
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(C.port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    check(bind(socket_fd, (const struct sockaddr *) &addr, sizeof(addr)) != 0, "Error: remote socket bind");
    //check(listen(socket_fd, SOMAXCONN) !=0, "Error: remote socket listen");


    return socket_fd;
}

void parse_arguments(int argc, char **argv) {
    if(argc != 3){
        printf("Wrong number of arguments\n");
        exit(-1);
    }
    char* to_int_error;
    long port_val = strtol(argv[1], &to_int_error, 10);
    if(errno == ERANGE || 0 != strcmp(to_int_error, "\0") || port_val < 0 || port_val > 20000) {
        printf("Wrong port number\n");
        exit(-1);
    }
    C.port = (uint16_t) port_val;
    C.socket_path = argv[2];
}

void cleanup(){
    for(int i = 0 ; i <N_CLIENTS; i++){
        if( C.clients[i].client_state == CONNECTED){
            FD_CLR(C.clients[i].fd, &C.descriptors);
            close(C.clients[i].fd);
        }
        FD_CLR(C.unix_socket, &C.descriptors);
        close(C.unix_socket);
        FD_CLR(C.inet_socket, &C.descriptors);
        close(C.inet_socket);
        unlink(C.socket_path);
    }

}

void exit_sig(int sig);
void setup_program() {
    signal(SIGINT, exit_sig);
    atexit(cleanup);
}

void exit_sig(int sig) {
    printf("Exit\n");
    exit(0);
}
