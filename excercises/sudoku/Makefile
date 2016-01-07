#VPATH = /home/chat/
LIB_SRC = ../../EventLoop.cc
BINARIES = batch server_basic

all: $(BINARIES)

include reactor.mk

batch: batch.cc sudoku.cc
server_basic: server_basic.cc sudoku.cc


.PHONY: clean
clean:
	rm -rf batch server_basic
