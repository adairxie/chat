#VPATH = /home/chat/
LIB_SRC = ../../EventLoop.cc 
BINARIES = server client

all: $(BINARIES)

include reactor.mk

server: server.cc codec.cc query.pb.cc
client: client.cc codec.cc query.pb.cc ../../TcpClient.cc
.PHONY: clean
clean:
	-rm -rf 
