#VPATH = /home/chat/
LIB_SRC = ../../EventLoop.cc
BINARIES = finger

all: $(BINARIES)

include reactor.mk

finger: main.cc

.PHONY: clean
clean:
	rm -rf finger
