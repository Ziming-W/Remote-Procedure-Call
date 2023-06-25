// partial source: Ziming's submission to COMP30023 assignment 1 [DataStructure.h]
// I have copied over some parts of it and modified in this file

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "dataStructure.h"


/* linked list node helpers*/

node_t* createNode(void* data, int32_t id){
    node_t* node = malloc(sizeof(node_t)); 
    assert(node != NULL); 
    node->data = data; 
    node->id = id; 
    node->next = NULL; 
    return node; 
}


/* linked list helpers*/

linkedList_t* createLinkedList(){
    linkedList_t* linkedList = (linkedList_t*)malloc(sizeof(linkedList_t));
    assert(linkedList != NULL); 
    linkedList->head = NULL; 
    linkedList->tail = NULL; 
    return linkedList;  
}

void insertAtLinkedListTail(linkedList_t* linkedList, node_t* newNode){
    assert(linkedList != NULL);
    assert(newNode != NULL); 
    newNode->next = NULL; 
    if(linkedList->tail == NULL){
        linkedList->head = linkedList->tail = newNode; 
    }
    else{
        linkedList->tail->next = newNode; 
        linkedList->tail = newNode; 
    }
}

/* registered function helper*/

function_t* createFunction(char* name, rpc_handler handler){
    function_t* newFunction = (function_t*)malloc(sizeof(*newFunction)); 
    assert(newFunction != NULL);
    //newFunction->name = name; 
    size_t name_len = strlen(name); 
    newFunction->name = (char*)malloc(name_len + 1); 
    strcpy(newFunction->name, name); 
    newFunction->handler = handler; 
    return newFunction; 
}
