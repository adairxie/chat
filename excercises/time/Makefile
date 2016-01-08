#VPATH = /home/chat/
LIB_SRC = ../../EventLoop.cc ../../TcpClient.cc
BINARIES = time timeclient

all: $(BINARIES)

include reactor.mk

time: time.cc main.cc
timeclient: timeclient.cc 

.PHONY: clean
clean:
	rm -rf time timeclient
