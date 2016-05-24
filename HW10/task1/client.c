#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <limits.h>
#include <unistd.h>
#include <sys/un.h>
#include <signal.h>
#include <sys/select.h>

#include "messages.h"

typedef union {
    struct sockaddr_in addr_in;
    struct sockaddr_un addr_un;
}addr_t;

socklen_t unix_address_size = sizeof(struct sockaddr_un);
socklen_t inet_address_size = sizeof(struct sockaddr_in);

void check(bool condition, char* msg) {if(condition){printf("%s\n", msg);}}


struct {
    char* user_name;
    char server_type;
    union {
        char* socket_path;
        struct {
            in_addr_t ip;
            uint16_t port;
        };
    };
} C;

request_t current_message;

struct {
    pthread_t thread;
    int client_socket;
    addr_t server_addr;
    fd_set descriptors;
    addr_t client_addr;
} P;

bool new_message = false;
bool finished = false;
void parse_input_args(int argc, char *argv[]);
void start_new_thread();
void* read_messages(void *data);
void communicate();
void setup_program();

void cleanup();

int main(int argc, char* argv[]) {
    parse_input_args(argc, argv);
    setup_program();
    start_new_thread();
    printf("Client started\n");

    communicate();
    pthread_join(P.thread, NULL);
    cleanup();
    printf("Exit\n");
    return 0;
}

int setup_communication();

int setup_local_communication();
int setup_remote_communication();
void communicate() {
    int fd = setup_communication();
    printf("Connection established\n");
    request_t broadcast;
    while(!finished) {
        fd_set descriptors;
        FD_ZERO(&descriptors);
        FD_SET(P.client_socket, &descriptors);
        printf("incoming message%s\n", P.server_addr.addr_un.sun_path);
        printf("Wait for select\n");
        check(select(P.client_socket+1, &descriptors, NULL, NULL, NULL) <= 0, "Error select");
        printf("Selected\n");

        if (FD_ISSET(P.client_socket, &descriptors)) {
            if (C.server_type == LOCAL) {
                if (recvfrom(P.client_socket, &current_message, sizeof(request_t), 0,
                             (struct sockaddr *) &P.server_addr.addr_un, &unix_address_size) > 0) {
                    printf("user: %s\t message: %s\n", broadcast.user_name, broadcast.message);
                }
            } else if (C.server_type == REMOTE) {
                if (recvfrom(P.client_socket, &current_message, sizeof(request_t), 0,
                             (struct sockaddr *) &P.server_addr.addr_in, &inet_address_size) > 0) {
                    printf("user: %s\t message: %s\n", broadcast.user_name, broadcast.message);
                }
            }
        }
    }

close(fd);
}

int setup_communication() {
    FD_ZERO(&P.descriptors);
    if(C.server_type == LOCAL){
        return setup_local_communication();
    } else {
        return setup_remote_communication();
    }
    FD_SET(P.client_socket, &P.descriptors);

}

int setup_remote_communication() {

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(C.port);
    addr.sin_addr.s_addr = C.ip;
    memcpy(&P.server_addr.addr_in, &addr, sizeof(struct sockaddr_in));

    P.client_socket = socket(AF_INET, SOCK_DGRAM, 0);
    check(P.client_socket <= 0, "Error: illegal file descriptor(remote server socket)");
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(C.port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    memcpy(&P.client_addr.addr_in, &addr, sizeof(struct sockaddr_in));
    check(bind(P.client_socket, (const struct sockaddr *) &addr, sizeof(addr)) != 0, "Error: unix socket bind");
    return P.client_socket;
}

int setup_local_communication() {

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, C.socket_path);
    memcpy(&P.server_addr.addr_un, &addr, sizeof(struct sockaddr_un));

    P.client_socket = socket(AF_UNIX, SOCK_DGRAM, 0);
    check(P.client_socket <= 0, "Error: illegal file descriptor(local server socket)");
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, C.user_name);
    memcpy(&P.client_addr.addr_un, &addr, sizeof(struct sockaddr_un));
    check(bind(P.client_socket, (const struct sockaddr *) &addr, sizeof(addr)) != 0, "Error: unix socket bind");
    printf("Sockets created\n");
    return P.client_socket;
}

void start_new_thread() {
    if (pthread_create(&P.thread, NULL, read_messages, NULL) != 0) {
        printf("Unable to create thread\n");
        exit(-1);
    }
}

void* read_messages(void *data){
    sleep(1);
    char* exit_com = "exit\n";
    while (true) {

        printf("Scan: \n");
        fgets(current_message.message, 1024, stdin);
        if(strcmp(exit_com, current_message.message) == 0) break;

        if(C.server_type == LOCAL)
            sendto(P.client_socket, &current_message, sizeof(request_t), 0,
                   (const struct sockaddr *) &P.server_addr.addr_un, sizeof(struct sockaddr_un));
        else if(C.server_type == REMOTE) {
            sendto(P.client_socket, &current_message, sizeof(request_t), 0,
                   (const struct sockaddr *) &P.server_addr.addr_in, sizeof(struct sockaddr_in));
        }



    }
    finished = true;
    return NULL;
}

bool check_socket(char *socket_path);

bool check_ip(char *ip_address, char* port);
void parse_input_args(int argc, char *argv[]) {
    if (argc<3 || (argv[2][0] != LOCAL && argv[2][0] != REMOTE) ||
        (argc == 4 && argv[2][0] != LOCAL) ||
        (argc == 5 && argv[2][0] != REMOTE)) {
        printf("Wrong number of arguments\n");
        exit(-1);
    }

    C.user_name = argv[1];
    strcpy(current_message.user_name, argv[1]);

    if (argv[2][0] != LOCAL && argv[2][0] != REMOTE) {
        printf("Unknown type of server\n");
        exit(-1);
    }
    C.server_type = argv[2][0];

    if (C.server_type == LOCAL ){
        check_socket(argv[3]);
    } else if (C.server_type == REMOTE) {
        check_ip(argv[3], argv[4]);
    } else {
        printf("Server not found\n");
        exit(-1);
    }
}

bool check_ip(char *ip_address, char* port) {
    C.ip = inet_addr(ip_address);

    char* to_int_error;
    long port_val = strtol(port, &to_int_error, 10);

    if(C.ip == ( in_addr_t)(-1) ||
       (errno == ERANGE || 0 != strcmp(to_int_error, "\0") ||
        port_val < 0 || port_val > UINT16_MAX)){
        printf("Illegal ip or port\n");
        exit(-1);
    }
    C.port = (uint16_t) port_val;
    return  true;
}

bool check_socket(char *socket_path) {
    C.socket_path = socket_path;
    return true;
}

void cleanup(){
    FD_CLR(P.client_socket, &P.descriptors);
    close(P.client_socket);
    unlink(C.user_name);
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
