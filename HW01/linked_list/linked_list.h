#ifndef ZAD1_LINKED_LIST_H
#define ZAD1_LINKED_LIST_H

#include <stdbool.h>

typedef struct Node Node_t;
typedef struct List List_t;

struct Node{
    char* first_name;
    char* last_name;
    char* email;
    char* phone;
    char* address;
    char* date;
    Node_t* prev;
    Node_t* next;
};

struct List{
    Node_t* last;
    Node_t* first;
};


Node_t* new_node();

Node_t* new_node_init(char* first_name, char* last_name, char* email, char* phone, char* address, char* date);

Node_t* del_node(Node_t* node);

List_t* new_list();

List_t* del_list(List_t* list);

bool insert_after(Node_t* node, Node_t* new_node);

Node_t* remove_from_list(Node_t* node);

Node_t* find(List_t* list, int (*comp)(Node_t *, char*), char* arg);

void sort(List_t* list, int (*comp)(Node_t *, Node_t *));

#endif //ZAD1_LINKED_LIST_H
