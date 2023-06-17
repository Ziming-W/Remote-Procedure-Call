## Purpose
Remote Procedure Call protocol  
Project work for COMP30023 (Computer Systems) at Unimelb

## File Description
| File    | Description |
| -------- | ------- |
| rpc.c  | main rpc logic    |
| send_recv_rpc_data.c | send/receive rpc_data (refer to rpc.h) structure through the socket     |
| dataStructure.c    | data structure helpers    |
| protocol.txt| protocol description|
|client.c|a very simple rpc test client|
|server.c |a very simple rpc test server|

## Complie
(Notice: some complied artifacts in Makefile are for marking purpose)
### Main Artifacts
    make
### Test client/server
    make
    make cs

## Run test client/server after compiling
Terminal window 1

    ./server

Terminal window 2

    ./client

## Sample test client/server log
server terminal window

    add2: arguments 127 and 127
    multi2: arguments 0 and 100
    multi2: arguments 1 and 100

client terminal window

    ERROR: Function add3 does not exist
    successfully found multi2 on server
    successfully found add2 on server
    Result of adding 127 and 127: 254
    Result of multipling 0 and 100: 0
    Result of multipling 1 and 100: 100