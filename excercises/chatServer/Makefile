#VPATH = /home/chat/
LIB_SRC = ../../EventLoop.cc 
BINARIES = server_threaded chatClient

all: $(BINARIES)

include reactor.mk

#chatServer: server.cc 
chatClient: client.cc ../../TcpClient.cc
#loadtest: loadtest.cc ../../TcpClient.cc

server_threaded: server_threaded.cc
#server_threaded_efficient: server_threaded_efficient.cc
#server_threaded_highperformance: server_threaded_highperformance.cc
.PHONY: clean
clean:
	-rm -rf loadtest server_threaded_highperformance
