// partial source: Ziming's submission to COMP30023 assignment 1 [DataStructure.h]
// I have copied over some parts of it and modified in this file
#ifndef DATA_STRUCTURE_H
#define DATA_STRUCTURE_H

# include "rpc.h"
# include <string.h>
# include <stdint.h>

/* purpose of this file: dataStructure related to linked list, nodes, and functions*/

// abstract linked list
typedef struct node node_t;

struct node {
    void* data;
    int32_t id; 
    node_t* next;
};

typedef struct {
    node_t* head;
    node_t* tail;
} linkedList_t;

// register function
typedef struct function function_t; 
struct function{
    char* name; 
    rpc_handler handler; 
}; 

node_t* createNode(void* data, int32_t id); 
linkedList_t* createLinkedList(); 
void insertAtLinkedListTail(linkedList_t* linkedList, node_t* newNode); 
function_t* createFunction(char* name, rpc_handler handler); 


#endif
