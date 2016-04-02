#include <stdlib.h>
#include <stdio.h>

#include "linked_list.h"


Node_t* new_node(){
    Node_t* newNode = calloc(1, sizeof(Node_t));
    return newNode;
}

Node_t* new_node_init(char* first_name, char* last_name, char* email, char* phone, char* address, char* date){
    Node_t* result = new_node();
    result->first_name = first_name;
    result->last_name = last_name;
    result->email = email;
    result->phone = phone;
    result->address = address;
    result->date = date;

    return result;
}

Node_t* del_node(Node_t* node){
    if(node == NULL) return node;
    if(node->next==NULL && node->prev==NULL){
        free(node);
        return NULL;
    } else {
        return node;
    }
}

List_t* new_list(){
    List_t* list = malloc(sizeof(List_t));

    list->first = new_node();
    list->last = new_node();
    list->first->next = list->last;
    list->last->prev =  list->first;
    return list;
}

List_t* del_list(List_t* list){
    if(list == NULL || list->first == NULL) return list;
    Node_t* current = list->first;

    while(current != NULL){
        Node_t* deleted = current;
        current = current->next;
        deleted->next = NULL;
        deleted->prev = NULL;
        del_node(deleted);
    }

    free(list);
    return NULL;
}


bool insert_after(Node_t* node, Node_t* new_node){
    if(node == NULL || new_node == NULL || node->next == NULL ||
       new_node->next != NULL || new_node->prev != NULL) return false;

    new_node->prev = node;
    new_node->next = node->next;
    node->next->prev = new_node;
    node->next = new_node;
    return true;
}

Node_t* remove_from_list(Node_t* node){
    if(node == NULL || node->next == NULL || node->prev == NULL) return NULL;

    node->next->prev = node->prev;
    node->prev->next = node->next;
    node->next = NULL;
    node->prev = NULL;

    return node;
}

Node_t* find(List_t* list, int (*comp)(Node_t *, char*), char* arg){
    if(list == NULL || list->first == NULL) return NULL;
    Node_t* current = list->first->next;

    while(current != NULL && current->next != NULL){	//ignore sentinels
        if(comp(current,arg)==0) return current;
        current=current->next;
    }
    return NULL;
}

void sort(List_t* list, int (*comp)(Node_t *, Node_t *)){
    if(list == NULL || list->first == NULL) return;
    Node_t* current = list->first->next->next;//start with second element

    while(current != NULL && current->next != NULL){ //don't check sentinel
        Node_t* insert = current;
        current = current->next;
        while(comp(insert->prev, current->prev) > 0 && insert->prev->prev != NULL){
            insert = insert->prev;
        }

        Node_t* insert_prev = insert->prev;
        insert = remove_from_list(current->prev);
        insert_after(insert_prev,insert);
    }
}

