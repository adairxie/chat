#VPATH = /home/chat/
LIB_SRC = ../../EventLoop.cc
BINARIES = discard

all: $(BINARIES)

include reactor.mk

discard: discard.cc main.cc

.PHONY: clean
clean:
	rm -rf discard
