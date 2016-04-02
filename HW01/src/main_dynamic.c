#include <dlfcn.h>
#include <stdio.h>
#include <linked_list.h>
#include <string.h>
#include <time.h>
#include <sys/resource.h>

void print_list(List_t* list);
void print_node(Node_t* node);
int comp_by_last_name(Node_t* node, char* name);
int comp_nodes_by_last_name(Node_t* node, Node_t* node2);

int main(int argc, char const *argv[])
{
    struct rusage rusage;
    clock_t my_clock;

    getrusage(RUSAGE_SELF, &rusage);
    my_clock = clock();
    double r1 = (double)my_clock / CLOCKS_PER_SEC;
    double u1 = (double)rusage.ru_utime.tv_sec + (double)rusage.ru_utime.tv_usec / (double)10e6;
    double s1 = (double)rusage.ru_stime.tv_sec + (double)rusage.ru_stime.tv_usec / (double)10e6;

    void* handle = dlopen("../build/lib/liblinked_list_shared.so", RTLD_NOW);


    Node_t* (*new_node_init)(char*, char*, char*, char*, char*, char*)=dlsym(handle, "new_node_init");
    Node_t* (*del_node)(Node_t*)=dlsym(handle, "del_node");
    List_t* (*new_list)()=dlsym(handle, "new_list");
    List_t* (*del_list)(List_t*)=dlsym(handle, "del_list");
    bool (*insert_after)(Node_t*, Node_t*)=dlsym(handle, "insert_after");
    Node_t* (*remove_from_list)(Node_t*)=dlsym(handle, "remove_from_list");
    Node_t* (*find)(List_t*, int (*comp)(Node_t*, char*), char*)=dlsym(handle, "find");
    void (*sort)(List_t*, int (*comp)(Node_t *, Node_t *))=dlsym(handle, "sort");


    List_t* list = new_list();
    Node_t* n1 = new_node_init("name_1", "last_name_1", "email_1", "phone_1", "address_1", "date_1");
    Node_t* n2 = new_node_init("name_2", "last_name_2", "email_2", "phone_2", "address_2", "date_2");
    Node_t* n3 = new_node_init("name_3", "last_name_3", "email_3", "phone_3", "address_3", "date_3");

    insert_after(list->first, n1);
    insert_after(list->first, n3);
    insert_after(list->first, n2);

    getrusage(RUSAGE_SELF, &rusage);
    my_clock = clock();
    double r2 = (double)my_clock / CLOCKS_PER_SEC;
    double u2 = (double)rusage.ru_utime.tv_sec + (double)rusage.ru_utime.tv_usec / (double)10e6;
    double s2 = (double)rusage.ru_stime.tv_sec + (double)rusage.ru_stime.tv_usec / (double)10e6;

    printf("list with 3 elements:\n");
    print_list(list);
    printf("\n");

    insert_after(n2, n1);
    printf("insert already existing element (operation should fail)\n");
    print_list(list);
    printf("\n");

    insert_after(n2, NULL);
    printf("insert NULL (operation should fail)\n");
    print_list(list);
    printf("\n");

    Node_t* found = find(list, comp_by_last_name, "last_name_2");
    printf("find node with lastname=last_name_2\n");
    print_node(found);
    printf("\n");

    found = find(list, comp_by_last_name, "last_name_9");
    printf("find node with lastname=last_name_9 if not exists\n");
    print_node(found);
    printf("\n");

    char* null_string = NULL;
    found = find(list, comp_by_last_name, null_string);
    printf("find node with lastname=NULL\n");
    print_node(found);
    printf("\n");


    Node_t* n4 = new_node_init("name_4", "last_name_4", "email_4", "phone_4", "address_4", "date_4");
    Node_t* n5 = new_node_init("name_5", "last_name_5", "email_5", "phone_5", "address_5", "date_5");
    Node_t* n6 = new_node_init("name_6", "last_name_6", "email_6", "phone_6", "address_6", "date_6");
    insert_after(list->first, n4);
    insert_after(n1, n5);
    insert_after(list->first, n6);
    printf("add some elements:\n");
    print_list(list);
    printf("\n");

    getrusage(RUSAGE_SELF, &rusage);
    my_clock = clock();
    double r3 = (double)my_clock / CLOCKS_PER_SEC;
    double u3 = (double)rusage.ru_utime.tv_sec + (double)rusage.ru_utime.tv_usec / (double)10e6;
    double s3 = (double)rusage.ru_stime.tv_sec + (double)rusage.ru_stime.tv_usec / (double)10e6;

    printf("after sort:\n");
    sort(list, comp_nodes_by_last_name);
    print_list(list);
    printf("\n");

    printf("remove node_2 from list:\n");
    remove_from_list(n2);
    del_node(n2);
    print_list(list);
    printf("\n");

    list = del_list(list);
    printf("deleted list:\n");
    print_list(list);
    printf("\n");

    n1 = new_node_init("name_1", "last_name_1", "email_1", "phone_1", "address_1", "date_1");
    printf("insert_after(NULL, node) returns: %d (false)\n", insert_after(NULL, n1));
    print_node(n1);
    printf("\n");

    n1 = del_node(n1);
    printf("del_node(n1): (n1 == NULL) == %d (true)\n", n1==NULL);
    print_node(n1);
    printf("\n");

    n1 = new_node_init("name_1", "last_name_1", "email_1", "phone_1", "address_1", "date_1");
    printf("insert_after(node, NULL) returns: %d (false)\n", insert_after(n1, NULL));
    printf("\tn1");
    print_node(n1);
    printf("\tn1->next");
    print_node(n1->next);
    printf("\n");

    n1 = del_node(n1);


    dlclose(handle);



    getrusage(RUSAGE_SELF, &rusage);
    my_clock = clock();
    double r4 = (double)my_clock / CLOCKS_PER_SEC;
    double u4 = (double)rusage.ru_utime.tv_sec + (double)rusage.ru_utime.tv_usec / (double)10e6;
    double s4 = (double)rusage.ru_stime.tv_sec + (double)rusage.ru_stime.tv_usec / (double)10e6;


    printf("1): r:%.20f u:%.20f s:%.20f \n",r1,u1,s1);
    printf("2): r:%.20f u:%.20f s:%.20f \n",r2,u2,s2);
    printf("3): r:%.20f u:%.20f s:%.20f \n",r3,u3,s3);
    printf("4): r:%.20f u:%.20f s:%.20f \n",r4,u4,s4);

    printf("1->2: r:%.20f u:%.20f s:%.20f \n",r2-r1,u2-u1,s2-s1);
    printf("2->3: r:%.20f u:%.20f s:%.20f \n",r3-r2,u3-u2,s3-s2);
    printf("3->4: r:%.20f u:%.20f s:%.20f \n",r4-r3,u4-u3,s4-s3);
    printf("1->4: r:%.20f u:%.20f s:%.20f \n",r4-r1,u4-u1,s4-s1);

    return 0;
}

int comp_by_last_name(Node_t* node, char* name){
    if(node == NULL || node->last_name == NULL || name == NULL) return -1;
    return strcmp(node->last_name, name);
}

int comp_nodes_by_last_name(Node_t* node, Node_t* node2){
    if(node == NULL || node2 == NULL) return 0;
    if(node->last_name == NULL && node2->last_name == NULL) return 0;
    if(node->last_name == NULL) return -1;
    if(node2->last_name == NULL) return 1;
    return strcmp(node->last_name, node2->last_name);
}

void print_list(List_t* list){
    if(list == NULL) {
        printf("\tNULL\n");
        return;
    }
    Node_t* current = list->first->next;

    while(current != NULL && current->next != NULL){
        printf("\telement: [%s, %s, %s, %s, %s, %s]\n", current->first_name,
               current->last_name, current->email, current->phone,
               current->address, current->date );
        current=current->next;
    }

}

void print_node(Node_t* node){
    if(node == NULL){
        printf("\tnode: NULL\n");
        return;
    }
    printf("\tnode: [%s, %s, %s, %s, %s, %s]\n", node->first_name,
           node->last_name, node->email, node->phone,
           node->address, node->date );
}