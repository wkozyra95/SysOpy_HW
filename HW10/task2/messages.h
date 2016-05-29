#ifndef HW10_MESSAGES_H
#define HW10_MESSAGES_H

#define MAX_SOCKET_PATH 100
#define MAX_USERNAME 100
#define MAX_MESSAGE 256

typedef enum {
    CONNECTED,
    DISCONNECTED
} connection_state;

typedef enum {
    INET,
    UNIX
} socket_type;

typedef struct {
    char username[MAX_USERNAME];
    char message_body[MAX_MESSAGE];
} request_t;

#endif //HW10_MESSAGES_H
