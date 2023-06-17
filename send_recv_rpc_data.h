#ifndef SEND_RECV_RPC_DATA_H
#define SEND_RECV_RPC_DATA_H

# include "rpc.h"

/* purpose of this file: functions for send/recv rpc_data through socket*/

rpc_data* recv_rpc_data(int sockfd_comm); 
int send_rpc_data(int sockfd_comm, rpc_data* send_data); 


#endif