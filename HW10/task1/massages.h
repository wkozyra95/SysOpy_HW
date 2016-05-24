#ifndef HW10_MASSAGES_H
#define HW10_MASSAGES_H

#define LOCAL 'A'
#define REMOTE 'B'

typedef enum {
    DISCONNECTED,
    CONNECTED
} state_t;

typedef enum {
    UNIX,
    INET
} socket_type;

typedef struct massage{
    char user_name[5];
    char massage[5];
} request_t;


#endif //HW10_MASSAGES_H
