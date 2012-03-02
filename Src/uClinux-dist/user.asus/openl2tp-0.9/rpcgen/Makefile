
CFLAGS=-D_= -D_RPC_THREAD_SAFE_

SRCS.o=	rpc_main.o rpc_hout.o rpc_cout.o rpc_parse.o \
	      rpc_scan.o rpc_util.o rpc_svcout.o rpc_clntout.o \
	      rpc_tblout.o rpc_sample.o

rpcgen: $(SRCS.o)
	$(CC) -o $@ $^ -lc

clean:
	rm -f $(SRCS.o) rpcgen
