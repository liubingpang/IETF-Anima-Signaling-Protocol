all : Client Server
complier=g++
CFLAGS=-Wall -pedantic -g -c
LFLAGS=-Wall -pedantic
MKDIR_P=mkdir -p
OUT_DIR=bin

$(OUT_DIR):
	$(MKDIR_P) $(OUT_DIR)

Client : $(OUT_DIR) main_client_demo.o Client.o Client_fsm_funcs.o Client_TCP.o ServerSession.o msg.o BaseNegotiator.o Errno.o Server.o Option.o
	$(complier) -o $(OUT_DIR)/Client main_client_demo.o Client.o Client_fsm_funcs.o Client_TCP.o ServerSession.o msg.o BaseNegotiator.o Errno.o Server.o Option.o $(LFLAGS) -lpthread

Server : $(OUT_DIR) main_server_demo.o Client.o Client_fsm_funcs.o ServerSession.o msg.o BaseNegotiator.o Errno.o Server.o Option.o
	$(complier) -o $(OUT_DIR)/Server main_server_demo.o Client.o Client_fsm_funcs.o ServerSession.o msg.o BaseNegotiator.o Errno.o Server.o Option.o $(LFLAGS) -lpthread

main_c.o : main_client_demo.cpp Client.h
	$(complier) -c main_client_demo.cpp Client.h $(CFLAGS)

main_s.o : main_server_demo.cpp Server.h
	$(complier) -c main_server_demo.cpp Server.h $(CFLAGS)

Client.o : Client.cpp Client.h BaseNegotiator.h Option.h
	$(complier) -c Client.cpp Client.h BaseNegotiator.h Option.h $(CFLAGS)

Client_fsm_funcs.o : Client_fsm_funcs.cpp Client.h Option.h
	$(complier) -c Client_fsm_funcs.cpp Client.h Option.h $(CFLAGS)

Client_TCP.o : Client_TCP.cpp Client.h Option.h
	$(complier) -c Client_TCP.cpp Client.h Option.h $(CFLAGS)

Server.o : Server.cpp Server.h ServerSession.h Option.h
	$(complier) -c Server.cpp Server.h ServerSession.h Option.h $(CFLAGS)

ServerSession.o : ServerSession.cpp ServerSession.h BaseNegotiator.h common_structs.h Option.h
	$(complier) -c ServerSession.cpp ServerSession.h BaseNegotiator.h common_structs.h Option.h $(CFLAGS)

msg.o : msg.cpp msg.h Errno.h
	$(complier) -c msg.cpp msg.h Errno.h $(CFLAGS)

BaseNegotiator : BaseNegotiator.cpp BaseNegotiator.h msg.h
	$(complier) -c BaseNegotiator.cpp BaseNegotiator.h msg.h $(CFLAGS)

Errno.o : Errno.cpp Errno.h
	$(complier) -c Errno.cpp Errno.h $(CFLAGS)

Option.o : Option.cpp Option.h
	$(complier) -c Option.cpp Option.h $(CFLAGS)

clean : 
	rm *.o
	rm *.gch
	rm -rf $(OUT_DIR)
