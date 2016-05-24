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

#include "massages.h"

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

request_t current_massage;

struct {
    pthread_t thread;
    int com_socket;
    int reg_socket;
} P;

bool new_message = false;
bool finished = false;
void parse_input_args(int argc, char *argv[]);
void start_new_thread();
void* read_massages(void *data);
void communicate();


int main(int argc, char* argv[]) {
    parse_input_args(argc, argv);
    start_new_thread();
    printf("Client started\n");

    communicate();
    pthread_join(P.thread, NULL);
    printf("Exit\n");
    return 0;
}

int setup_communication();
int setup_local_communication();
int setup_remote_communication();

void communicate() {
    int fd = setup_communication();
    printf("Connection established\n");
    struct response server_response;
    while(!finished){

        if ( write(fd, &current_massage, sizeof(current_massage)) == -1){
            printf("Error while sending data\n");
        }
        printf("Massage sent\n");
        if (read(fd, &server_response, sizeof(server_response)) == -1){
            printf("Error while sending data\n");
        }
        printf("response: %s\n", server_response.correct);
        new_message = false;
    }
    close(fd);
}

int setup_communication() {
    if(C.server_type == LOCAL){
        return setup_local_communication();
    } else {
        return setup_remote_communication();
    }

}

int setup_remote_communication() {
    printf("Create inet_socket ip: %d port: %d\n", C.ip, C.port);
    P.reg_socket = socket(AF_INET, SOCK_DGRAM, 0);
    check(P.reg_socket <= 0, "Error: illegal file descriptor(remote server socket)");
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons( C.port);
    addr.sin_addr.s_addr = C.ip;
    int fd = connect(P.reg_socket, (const struct sockaddr *) &addr, sizeof(addr));
    check(fd != 0, "Error: illegal file descriptor(remote socket)");
    return P.reg_socket;
}

int setup_local_communication() {
    printf("Create unix_socket path: %s\n", C.socket_path);
    P.reg_socket = socket(AF_UNIX, SOCK_DGRAM, 0);
    check(P.reg_socket <= 0, "Error: illegal file descriptor(local server socket)");
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path,C.socket_path);
    int fd = connect(P.reg_socket, (const struct sockaddr *) &addr, sizeof(addr));
    check(fd != 0, "Error: illegal file descriptor(local socket)");
    return P.reg_socket;
}

void start_new_thread() {

    if (pthread_create(&P.thread, NULL, read_massages, NULL) != 0) {
        printf("Unable to create thread\n");
        exit(-1);
    }

}

void* read_massages(void* data){
    sleep(1);
    char* input = malloc(1024);
    char* exit_com = "exit\n";
    while (true) {

        printf("Scan: \n");
        fgets(input, 1024, stdin);
        if(strcmp(exit_com, input) == 0) break;
        strcpy(current_massage.massage, input);
        new_message = true;
        while(new_message);

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
    strcpy(current_massage.user_name, argv[1]);

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
