CC=cc
RPC_SYSTEM=rpc.o
RPC_ALONE=rpcAlone.o
DATA_STRUCTURE=dataStructure.o
SEND_RECV_RPC_DATA=send_recv_rpc_data.o

RPC-SERVER=rpc-server
RPC-CLIENT=rpc-client
CI-SERVER=ci-server
CI-CLIENT=ci-client

CLIENT=client
SERVER=server
SRC=rpc.c dataStructure.c

CFLAGS=-Wall

.PHONY: format all

# all:$(RPC_ALONE) $(DATA_STRUCTURE) $(RPC_SYSTEM) $(RPC-SERVER) $(RPC-CLIENT)
all:$(RPC_ALONE) $(DATA_STRUCTURE) $(SEND_RECV_RPC_DATA) $(RPC_SYSTEM)
ci: $(CI-SERVER) $(CI-CLIENT)
cs: $(CLIENT) $(SERVER)
kk: clean $(RPC_ALONE) $(DATA_STRUCTURE) $(RPC_SYSTEM) $(RPC-CLIENT) $(RPC-SERVER) $(CLIENT) $(SERVER)

$(RPC_SYSTEM): rpc.c rpc.h dataStructure.h dataStructure.c send_recv_rpc_data.c send_recv_rpc_data.h
	ld -r rpcAlone.o dataStructure.o send_recv_rpc_data.o -o rpc.o

$(RPC_ALONE): $(SRC) rpc.h dataStructure.h send_recv_rpc_data.h
	$(CC) $(CFLAGS) -c rpc.c -o rpcAlone.o

$(DATA_STRUCTURE): $(SRC) rpc.h dataStructure.h send_recv_rpc_data.h
	$(CC) $(CFLAGS) -c dataStructure.c -o dataStructure.o

$(SEND_RECV_RPC_DATA): $(SRC) rpc.h dataStructure.h send_recv_rpc_data.h
	$(CC) $(CFLAGS) -c send_recv_rpc_data.c -o send_recv_rpc_data.o

$(RPC-CLIENT): rpc-client.c rpc.c rpc.h dataStructure.c dataStructure.h
	$(CC) $(CFLAGS) -o $@ $^  

$(RPC-SERVER): rpc-server.c rpc.c rpc.h dataStructure.c dataStructure.h
	$(CC) $(CFLAGS) -o $@ $^ 

$(CLIENT): client.c rpc.c rpc.h dataStructure.c dataStructure.h send_recv_rpc_data.c send_recv_rpc_data.h
	$(CC) $(CFLAGS) -o $@ $^  

$(SERVER): server.c rpc.c rpc.h dataStructure.c dataStructure.h send_recv_rpc_data.c send_recv_rpc_data.h
	$(CC) $(CFLAGS) -o $@ $^

$(CI-SERVER): rpc.o
	$(CC) $(CFLAGS) -o ci-server server.a rpc.o

$(CI-CLIENT): rpc.o
	$(CC) $(CFLAGS) -o ci-client client.a rpc.o

format:
	clang-format -style=file -i *.c *.h

clean:
	rm -f rpc.o rpcAlone.o dataStructure.o $(RPC-CLIENT) $(RPC-SERVER) *.o $(CLIENT) $(SERVER)

clean-ci:
	rm -f ci-client $(CI-SERVER) $(CI-CLIENT)
