#ifndef HW10_MASSAGES_H
#define HW10_MASSAGES_H

#define LOCAL 'A'
#define REMOTE 'B'

typedef enum {
    DISCONNECTED,
    CONNECTED
} state_t;

typedef enum {
    NO_MESSAGE,
    UNIX,
    INET
} message_status;

typedef struct massage{
    char user_name[100];
    char massage[1024];
} request_t;

struct response{
    char correct[100];
};

#endif //HW10_MASSAGES_H
