#VPATH = /home/chat/
LIB_SRC = ../../EventLoop.cc
BINARIES = echo

all: $(BINARIES)

include reactor.mk

echo: echo.cc main.cc

.PHONY: clean
clean:
	rm -rf echo
