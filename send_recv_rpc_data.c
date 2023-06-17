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
#include <endian.h>

#include "send_recv_rpc_data.h"
#define MAX_DATA2_LEN 100000

/* receive rpc_data through socket, return NULL if fails, return rpc_data* if success*/
rpc_data* recv_rpc_data(int sockfd_comm){
    //receive flag, if -1, quit directly
    int32_t flag, flag_nbo; 
    if(recv(sockfd_comm, &flag_nbo, sizeof(flag_nbo), 0) <=0){
        perror("recv rpc_data flag"); 
        return NULL; 
    }
    flag = ntohl(flag_nbo); 
    if(flag == -1){
        perror("recv rpc_data invalid flag, early exit"); 
        return NULL; 
    }
    
    //receive data 1
    int64_t data1, data1_nbo; 
    if(recv(sockfd_comm, &data1_nbo, sizeof(data1_nbo), 0) <= 0){
        perror("recv rpc_data data1"); 
        return NULL; 
    }
    data1 = be64toh(data1_nbo); 

    //receive data 2 len
    uint32_t data2_len_nbo; 
    size_t data2_len; 
    if(recv(sockfd_comm, &data2_len_nbo, sizeof(data2_len_nbo), 0) <= 0){
        perror("recv rpc_data data2_len"); 
        return NULL; 
    }
    data2_len = (size_t)(ntohl(data2_len_nbo)); 

    //receive data2 only if data2_len is greater than 0
    void* data2 = NULL;
    if(data2_len > 0){ 
        data2 = malloc(data2_len);
        if(recv(sockfd_comm, data2, data2_len, 0) <= 0){
            free(data2); 
            perror("recv rpc_data data2"); 
            return NULL; 
        }
    }

    //construct rpc_data
    rpc_data* recv_data = (rpc_data*)malloc(sizeof(*recv_data));
    if(recv_data == NULL){
        perror("can't allocate recv_data\n"); 
        return NULL; 
    }
    recv_data->data1 = data1; 
    recv_data->data2_len = data2_len; 
    recv_data->data2 = data2; 

    return recv_data; 
}

/* send rpc_data through socket, return -1 if fails, return 0 if success */
int send_rpc_data(int sockfd_comm, rpc_data* send_data){
    // verify the integrity of rpc_data, and send flag
    int32_t flag = 0; 
    if(send_data == NULL
    || (send_data->data2_len < 0)
    || (send_data->data2_len >= MAX_DATA2_LEN)
    || (send_data->data2_len > 0 && send_data->data2 == NULL)
    || (send_data->data2_len == 0 && send_data->data2 != NULL)
    ){
        flag = -1; 
    }
    int32_t flag_nbo = htonl(flag);
    if(send(sockfd_comm, &flag_nbo, sizeof(flag_nbo), 0) <= 0){
        perror("send rpc_data flag"); 
        return -1; 
    }

    // based on the verification, report error then quit, or quit directly
    // if rpc_data is NULL, quit directly
    if(send_data == NULL){
        perror("send rpc_data null rpc_data"); 
        return -1; 
    } 
    //if data2 overflow, report error to stderr then quit
    if(send_data->data2_len >= MAX_DATA2_LEN){
        fprintf(stderr, "Overlength error\n"); 
        return -1; 
    }
    // if other error, quit
    if(flag == -1){
        perror("invalid rpc_data, early exit"); 
        return -1; 
    }

    //send data1
    int64_t data1_nbo = htobe64(send_data->data1); 
    if(send(sockfd_comm, &data1_nbo, sizeof(data1_nbo), 0) <= 0){
        perror("send rpc_data data1"); 
        return -1; 
    }
   
    //send data2 len
    uint32_t data2_len_nbo; 
    data2_len_nbo = htonl(send_data->data2_len); 
    if(send(sockfd_comm, &data2_len_nbo, sizeof(data2_len_nbo), 0) <= 0){
        perror("send rpc_data data2_len"); 
        return -1; 
    }

    //send data2 only if data2_len is greater than 0
    if(send_data->data2_len > 0){
        if(send(sockfd_comm, send_data->data2, send_data->data2_len, 0) <= 0){
            perror("send rpc_data data2"); 
            return -1; 
        }
    }
    return 0; 
}