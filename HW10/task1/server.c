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

#define N_CLIENTS 100
int max(int a, int b){ return a>b?a:b; }
void check(bool condition, char* msg) {if(condition){printf("%s\n", msg);}}

socklen_t unix_address_size = sizeof(struct sockaddr_un);
socklen_t inet_address_size = sizeof(struct sockaddr_in);


typedef struct  {
    state_t client_state;
    time_t timestamp;
    socket_type type;
    union {
        struct sockaddr_in addr_in;
        struct sockaddr_un addr_un;
    } addr;
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

void handle_message(struct sockaddr *fd, int i, request_t *pMassage);
void register_client(struct sockaddr *server_socket, socket_type param);
void delete_unactive_clients();

void unregister_client(struct sockaddr* addr);

void communicate() {

    request_t request;
    struct sockaddr_in inet_addr;
    struct sockaddr_un unix_addr;
    ssize_t recv_state;
    while(true) {

        check(select(C.max_fd+1, &C.descriptors, NULL, NULL, NULL) < 0, "Select: new messages");
        printf("Select----------------------------\n");

        recv_state = recvfrom(C.unix_socket, &request, sizeof(request), MSG_DONTWAIT,
                              (struct sockaddr *) &unix_addr, &unix_address_size);
        if(recv_state > 0){
            handle_message((struct sockaddr *) &unix_addr, C.unix_socket, &request);
        } else if (recv_state == 0) {
            unregister_client((struct sockaddr*) &unix_addr);
        }

        recv_state = recvfrom(C.inet_socket, &request, sizeof(request), MSG_DONTWAIT,
                              (struct sockaddr *) &inet_addr, &inet_address_size);
        if(recv_state > 0){
            handle_message((struct sockaddr *) &inet_addr, C.inet_socket, &request);
        } else if (recv_state == 0) {
            unregister_client((struct sockaddr*) &inet_addr);
        }

        delete_unactive_clients();
    }
}

void unregister_client(struct sockaddr* addr) {
    printf("Unregistered client\n");
    for (int i = 0; i< N_CLIENTS; i++){
        if(C.clients[i].client_state == CONNECTED){
            if(C.clients[i].type == UNIX &&
               memcmp(&C.clients[i].addr.addr_un, addr, sizeof(struct sockaddr_un))){
                C.clients[i].client_state = DISCONNECTED;
                break;
            } else if (C.clients[i].type == INET &&
                       memcmp(&C.clients[i].addr.addr_in, addr, sizeof(struct sockaddr_in))){
                C.clients[i].client_state = DISCONNECTED;
                break;
            }
        }
    }
}


void handle_message(struct sockaddr *addr, int socket, request_t *request) {
    printf("Received massage:%s %s\n",request->user_name, request->massage);
    bool new_client = true;
    for (int i = 0; i < N_CLIENTS; i++) {
        request_t r;
        memcpy(&r, request, sizeof(request_t));
        if (C.clients[i].client_state == DISCONNECTED) continue;
        //printf("Connected client counter\n");
        if(C.clients[i].type == UNIX){

            if(memcmp(&C.clients[i].addr.addr_un, addr, sizeof(struct sockaddr_un)) == 0){
                new_client = false;
                C.clients[i].timestamp = time(NULL);
                //printf("unix client updated\n");
            } else {
                printf("Unix socket send %s\t %s to %s\n", r.user_name, r.massage, C.clients[i].addr.addr_un.sun_path);
                check(sendto(C.unix_socket, &r, sizeof(request_t), 0,
                             (const struct sockaddr *) &C.clients[i].addr.addr_un,
                             sizeof(struct sockaddr_un)) == -1, "Unable to send\n");
            }
        } else if(C.clients[i].type == INET) {
            if (memcmp(&C.clients[i].addr.addr_un, addr, sizeof(struct sockaddr_in)) == 0) {
                new_client = false;
                C.clients[i].timestamp = time(NULL);
            } else {
                check(sendto(C.inet_socket, &r, sizeof(request_t), 0,
                             (const struct sockaddr *) &C.clients[i].addr.addr_in,
                             sizeof(struct sockaddr_in)) == -1, "Unable to send\n");
            }
        }
    }
    if(new_client){
        register_client(addr, ((socket == C.unix_socket) ? UNIX : INET));
    }
}

void delete_unactive_clients() {
    for (int i = 0; i< N_CLIENTS; i++){
        if(C.clients[i].client_state == CONNECTED && time(NULL) - C.clients[i].timestamp > 30){
            C.clients[i].client_state = DISCONNECTED;
        }
    }
}

void register_client(struct sockaddr *addr, socket_type type) {
    int i;
    for(i = 0; i<N_CLIENTS; i++){
        if(C.clients[i].client_state == DISCONNECTED) break;
    }
    if(i == N_CLIENTS) return;
    printf("Client id%d\n", i);
    C.clients[i].client_state = CONNECTED;
    C.clients[i].timestamp = time(NULL);
    C.clients[i].type = type;
    if(type == UNIX)
        memcpy(&C.clients[i].addr.addr_un, addr, sizeof(struct sockaddr_un));
    else
        memcpy(&C.clients[i].addr.addr_in, addr, sizeof(struct sockaddr_in));
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
    int socket_fd = socket(AF_UNIX, SOCK_DGRAM, 0);

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, C.socket_path);

    check(bind(socket_fd, (const struct sockaddr *) &addr, sizeof(addr)) != 0, "Error: unix socket bind");
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
    FD_CLR(C.unix_socket, &C.descriptors);
    close(C.unix_socket);
    FD_CLR(C.inet_socket, &C.descriptors);
    close(C.inet_socket);
    unlink(C.socket_path);
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
