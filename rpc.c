#include <stdlib.h>

#include "rpc.h"
#include "dataStructure.h" 
#include "send_recv_rpc_data.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <stdio.h>
#include <pthread.h>
#include <stdint.h>

#define NONBLOCKING

// source: COMP30023 Week9 lab
// purpose: create a listening socket for server
// !!! modified by Ziming
int create_listening_socket(char* service) {
	int re, s, sockfd;
	struct addrinfo hints, *res;

	// Create address we're going to listen on (with given port number)
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET6;       // IPv6
	hints.ai_socktype = SOCK_STREAM; // Connection-mode byte streams
	hints.ai_flags = AI_PASSIVE;     // for bind, listen, accept
	// node (NULL means any interface), service (port), hints, res
	s = getaddrinfo(NULL, service, &hints, &res);
	if (s != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
		//exit(EXIT_FAILURE);
        return -1; 
	}

	// Create socket
	sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (sockfd < 0) {
		perror("socket");
		//exit(EXIT_FAILURE);
        return -1; 
	}

	// Reuse port if possible
	re = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &re, sizeof(int)) < 0) {
		perror("setsockopt");
		//exit(EXIT_FAILURE);
        return -1; 
	}
	// Bind address to the socket
	if (bind(sockfd, res->ai_addr, res->ai_addrlen) < 0) {
		perror("bind");
		//exit(EXIT_FAILURE);
        return -1; 
	}
	freeaddrinfo(res);

	return sockfd;
}

//RPC request type enum(call, find)
typedef enum{
    CALL_REQUEST = 1, 
    FIND_REQUEST = 2, 
}rpc_request_t; 

// thread args, server creates a thread for each client
typedef struct{
    rpc_server* srv; 
    int sockfd_comm; 
}thread_args_t; 

/* rpc_server state*/
struct rpc_server {
    int sockfd_listen; 
    linkedList_t* functionList; 
    int32_t nextNodeId; 
};

/* initial a rpc_server */
rpc_server *rpc_init_server(int port) {
    if(port < 0 || port > 65535){
        return NULL; 
    }
    // initial rpc_server
    rpc_server* server = (rpc_server*)malloc(sizeof(*server));
    if(!server){
        perror("malloc-rpc_init_server"); 
        return NULL; 
    } 
    char port_str[6]; // convert port to char* (port <= 65535)
    sprintf(port_str, "%d", port); 
    server->functionList = createLinkedList(); 
    server->nextNodeId = 0; 
    // create listening socket
    server->sockfd_listen = create_listening_socket(port_str); 
    if(server->sockfd_listen < 0){
        perror("can't create listening socket"); 
        return NULL; 
    }

    return server; 
}

/* register a function on rpc_server*/
int rpc_register(rpc_server *srv, char *name, rpc_handler handler) {
    if(!srv || !name || !handler){
        perror("can't register function cuz srv/name/handler is NULL"); 
        return -1; 
    }
    //check if there is a function with the same name has been registered already
    node_t* currNode = srv->functionList->head; 
    while(currNode != NULL){
        if(strcmp(((function_t*)(currNode->data))->name, name) == 0){
            ((function_t*)(currNode->data))->handler = handler; 
            return 1; 
        }
        currNode = currNode->next; 
    }
    //if not, create new node, and insert at end
    function_t* newFunction = createFunction(name, handler); 
    node_t* newFunctionNode = createNode(newFunction, srv->nextNodeId); 
    srv->nextNodeId++; 
    insertAtLinkedListTail(srv->functionList, newFunctionNode);
    return 1;  
}

/* rpc_server handles a find request*/
void rpc_server_handle_find(int sockfd_comm, rpc_server* srv){
    //receive function name length
    int32_t functionNameLen_nbo, functionNameLen;
    if(recv(sockfd_comm, &functionNameLen_nbo, sizeof(functionNameLen_nbo), 0) <= 0){
        perror("recv-rpc_server_handle_find"); 
        return; 
    }
    functionNameLen = ntohl(functionNameLen_nbo); 
    //receive function name
    char* functionName = (char*)malloc(functionNameLen + 1); 
    if(recv(sockfd_comm, functionName, functionNameLen, 0) <= 0){
        free(functionName);
        perror("recv-rpc_server_handle_find");
        return; 
    }
    functionName[functionNameLen] = '\0'; 
    //try to find this handle
    node_t* currNode = srv->functionList->head; 
    int32_t responseFlag = -1; 
    while(currNode != NULL){
        if(strcmp(functionName, ((function_t*)currNode->data)->name) == 0){
            responseFlag = currNode->id; 
            break; 
        }
        currNode = currNode->next; 
    }
    //send response flag (node id)
    int32_t responseFlag_nbo = htonl(responseFlag); 
    if(send(sockfd_comm, &responseFlag_nbo, sizeof(responseFlag_nbo), 0) < 0){
        free(functionName);
        perror("server send response flag"); 
        return; 
    }
    //free memory
    free(functionName); 
}

/* rpc_server handle call request*/
void rpc_server_handle_call(int sockfd_comm, rpc_server* srv){
    //receive rpc_data input
    rpc_data* input = recv_rpc_data(sockfd_comm); 
    if(input == NULL){
        perror("rpc_server_handle_call: didn't receive data"); 
        return; 
    }
    //receive node id
    int32_t id, id_nbo; 
    if(recv(sockfd_comm, &id_nbo, sizeof(id_nbo), 0) <= 0){
       perror("recv-rpc_server_handle_call"); 
       return; 
    }
    id = ntohl(id_nbo); 
    // iterate linked list to find the handler
    node_t* currNode = srv->functionList->head;
    rpc_handler handler = NULL; 
    while(currNode != NULL){
        if(currNode->id == id){
            handler = ((function_t*)currNode->data)->handler;
            break;  
        }
        currNode = currNode->next; 
    }
    // error checking, this is unlikely to happen due to spec, fix later
    if(handler == NULL){
        rpc_data_free(input); 
        return; 
    }
    //call function and get output
    rpc_data* output = handler(input); 
    //send response back
    if(send_rpc_data(sockfd_comm, output) < 0){
        perror("rpc_server_handle_call: can't send data");
    }
    //free memory 
    rpc_data_free(input); 
}

/* handle a thread's communication with a client*/
void* rpc_server_handle_thread(void* args){
    thread_args_t* thread_args = (thread_args_t*)args; 
    rpc_server* srv = thread_args->srv; 
    int sockfd_comm = thread_args->sockfd_comm; 
    while (1) {
        // process requestType
        int32_t requestType_int_nbo; 
        rpc_request_t* requestType = (rpc_request_t*)malloc(sizeof(*requestType));
        int recvCode = recv(sockfd_comm, &requestType_int_nbo, sizeof(requestType_int_nbo), 0); 
        if (recvCode < 0) {
            perror("recv-rpc_serve_all");
            free(requestType);
            break; 
        }
        else if(recvCode == 0){
            free(requestType); 
            break; 
        }
        *requestType = (rpc_request_t)ntohl(requestType_int_nbo); 
        // handle request
        if (*requestType == FIND_REQUEST) {
            rpc_server_handle_find(sockfd_comm, srv);
        } else if (*requestType == CALL_REQUEST) {
            rpc_server_handle_call(sockfd_comm, srv);
        } else {
            perror("unknown request type");
        }
        free(requestType);
    }
    //close connection
    close(sockfd_comm); 
    free(args); 
    return NULL; 
}

/* rpc_server's main loop */
void rpc_serve_all(rpc_server *srv) {
    if(!srv){
        perror("rpc_serve_all-empty srv"); 
        return; 
    }
    //start listen
    if(listen(srv->sockfd_listen, 10) < 0){
        perror("listen-rpc_serve_all"); 
        return; 
    }
    // main loop to accept client's request
    while(1){
        //accept request
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr); 
        int sockfd_comm = accept(srv->sockfd_listen, (struct sockaddr*)&client_addr, &client_addr_len); 
        if(sockfd_comm < 0){
            close(sockfd_comm); 
            perror("accept-rpc_serve_all"); 
            continue; 
        }
        //create new thread to handle this request
        pthread_t cl_thread; 
        thread_args_t* thread_args = (thread_args_t*)malloc(sizeof(*thread_args)); 
        thread_args->srv = srv; 
        thread_args->sockfd_comm = sockfd_comm; 
        if(pthread_create(&cl_thread, NULL, rpc_server_handle_thread, thread_args) != 0){
            perror("fails to create thread"); 
            close(sockfd_comm); 
            free(thread_args); 
        }
        else{
            pthread_detach(cl_thread); 
        }
    }
}

/* rpc_client's state*/
struct rpc_client {
    int sockfd_comm; 
};

/* rpc_handle, store only registered function node's id*/
struct rpc_handle {
    int32_t id; 
};

/* initial rpc_client */
rpc_client *rpc_init_client(char *addr, int port) {
    if(port < 0 || port > 65535){
        return NULL; 
    }
    // allocate memory
    rpc_client *client = (rpc_client *)malloc(sizeof(rpc_client));
    if (!client) {
        perror("malloc");
        return NULL;
    }
    //build socket
    client->sockfd_comm = socket(AF_INET6, SOCK_STREAM, 0);
    if (client->sockfd_comm < 0) {
        perror("socket");
        free(client);
        return NULL;
    }
    //connect to server
    struct sockaddr_in6 server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin6_family = AF_INET6;
    if (inet_pton(AF_INET6, addr, &server_addr.sin6_addr) < 0) {
        perror("inet_pton");
        close(client->sockfd_comm);
        free(client);
        return NULL;
    }
    server_addr.sin6_port = htons(port);
    if (connect(client->sockfd_comm, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        //close(client->sockfd_comm);
        free(client);
        return NULL;
    }
    return client;
}

/* rpc_client send find request to rpc_server*/
rpc_handle *rpc_find(rpc_client *cl, char *name) {
    if(!cl || !name){
        perror("rpc_find-cl or name is null"); 
        return NULL; 
    }
    //send request type
    rpc_request_t requestType = FIND_REQUEST; 
    int32_t requestType_nbo = htonl((int32_t)requestType); 
    if(send(cl->sockfd_comm, &requestType_nbo, sizeof(requestType_nbo), 0) <= 0){
        perror("send-rpc_find"); 
        return NULL; 
    }
    //send function name length
    int32_t len = strlen(name); 
    int32_t len_nbo = htonl(len); 
    if(send(cl->sockfd_comm, &len_nbo, sizeof(len_nbo), 0) <= 0){
        perror("send-functionNameLen"); 
        return NULL; 
    }
    //send function name
    if(send(cl->sockfd_comm, name, len, 0) <= 0){
        perror("send-functionName"); 
        return NULL; 
    }
    //receive flag (node id)
    int32_t flag, flag_nbo; 
    if(recv(cl->sockfd_comm, &flag_nbo, sizeof(flag_nbo), 0) <= 0){
        perror("recv-flag"); 
        return NULL; 
    }
    flag = ntohl(flag_nbo); 
    //check flag
    if(flag >= 0){
        rpc_handle* handle = (rpc_handle*)malloc(sizeof(*handle)); 
        handle->id = flag; 
        return handle; 
    }
    else{
        return NULL; 
    }
}

/* rpc_client send call request to rpc_server */
rpc_data *rpc_call(rpc_client *cl, rpc_handle *h, rpc_data *payload) {
    if(!cl || !h || !payload){
        return NULL; 
    }
    //send rpc_call request
    rpc_request_t requestType = CALL_REQUEST; 
    int32_t requestType_nbo = htonl((int32_t)requestType); 
    if(send(cl->sockfd_comm, &requestType_nbo, sizeof(requestType_nbo), 0) < 0){
        perror("rpc_call: fail to send requestType"); 
        return NULL; 
    }
    //send rpc_data to server
    if(send_rpc_data(cl->sockfd_comm, payload) < 0){
        perror("rpc_call: can't send rpc_data "); 
        return NULL; 
    }
    //send node id to server
    int32_t id_nbo = htonl(h->id); 
    if(send(cl->sockfd_comm, &(id_nbo), sizeof(id_nbo), 0) <= 0){
        perror("rpc_call: can't send node id to server"); 
        return NULL; 
    }
    //recv rpc_data output from server
    rpc_data* result_data = recv_rpc_data(cl->sockfd_comm); 
    if(result_data == NULL){
        perror("rpc_call: received NULL data from server"); 
        return NULL; 
    }
    return result_data; 
}

/* rpc_client closes connection , and free memory */
void rpc_close_client(rpc_client *cl) {
    if(cl == NULL){
        return; 
    }
    close(cl->sockfd_comm); 
    free(cl); 
}

/* free a rpc_data struct */
void rpc_data_free(rpc_data *data) {
    if (data == NULL) {
        return;
    }
    if (data->data2 != NULL) {
        free(data->data2);
    }
    free(data);
}
