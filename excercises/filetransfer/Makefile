#VPATH = /home/chat/
LIB_SRC = ../../EventLoop.cc 
BINARIES =  FileServer

all: $(BINARIES)

include reactor.mk

FileServer: download3.cc

.PHONY: clean
clean:
	rm -rf FileServer
