## Purpose
Remote Procedure Call protocol  
Project work for COMP30023 (Computer Systems) at Unimelb

## Patch logs
| Commit    |Date | Note |
| -------- | ------- | --------| 
| [Submitted Version](https://github.com/Ziming-W/Remote-Procedure-Call/commit/36c95e72dd28e261e12eb7dc952ac14a6544adbe)|-| 13/15, passed all visible tests, minor memory leak, failed small number of hidden test
|[Fixed bugs for hidden test](https://github.com/Ziming-W/Remote-Procedure-Call/commit/1edd8f2fc4dccf1b0385d41a4d22b005750d3d76)|2023-6-25|14/15 Fixed bugs regarding long function name
|[Fixed bugs for hidden tests](https://github.com/Ziming-W/Remote-Procedure-Call/commit/b389b3822546764ab77bb6d2769d342bd99fa3fb)|2023-6-25|14.05/15 Fixed bugs regarding bad port for client/server
## File Description
| File    | Description |
| -------- | ------- |
| rpc.c rpc.h | main rpc logic    |
| send_recv_rpc_data.c send_recv_rpc_data.h| send/receive rpc_data (refer to rpc.h) structure through the socket     |
| dataStructure.c dataStructure.h   | data structure helpers    |
| protocol.txt| protocol description|
|client.c|a very simple rpc test client|
|server.c |a very simple rpc test server|
|server.a client.a | advanced testing framework artifacts, the name "server/client" clashed with server.c, client.c, but they are not relevant with each other

## Complie
(Notice: some complied artifacts in Makefile are for marking purpose)
### Main Artifacts
    make

### Very Simple Test client/server
    make
    make cs

### Advanced marking frameworks
    make
    make ci

### clean main
    make clean

### clean advanced marking frameworks executable
    make clean-ci
    
## Run Very Simple test client/server after compiling
Server Terminal Window

    ./server

Client Terminal Window

    ./client

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

## Run advanced marking framework
`input: *.in`  
`sample output: *.out`

server terminal window

    ./ci-server < cases/1+1/server.in

client terminal window

    ./ci-client < cases/1+1/client.in

or check diff simultaneously

    ./ci-client < cases/1+1/client.in | diff - cases/1+1/client.out
