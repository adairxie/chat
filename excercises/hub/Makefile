#VPATH = /home/chat/
LIB_SRC = ../../EventLoop.cc  ../../FileUtil.cc
BINARIES = sub pub hub hub_threaded hub_threaded_efficient

all: $(BINARIES)

include reactor.mk

sub: sub.cc  pubsub.cc ../../TcpClient.cc ../../ProcessInfo.cc codec.cc
pub: pub.cc  pubsub.cc ../../TcpClient.cc ../../ProcessInfo.cc codec.cc 
hub: hub.cc  pubsub.cc ../../TcpClient.cc ../../ProcessInfo.cc codec.cc 
hub_threaded: hub_threaded.cc  pubsub.cc ../../TcpClient.cc ../../ProcessInfo.cc codec.cc 
hub_threaded_efficient: hub_threaded_efficient.cc  pubsub.cc ../../TcpClient.cc ../../ProcessInfo.cc codec.cc 

.PHONY: clean
clean:
	-rm -rf sub pub
