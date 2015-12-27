LIB_SRC = EventLoop.cc
BINARIES = echo

all: $(BINARIES)

include reactor.mk

echo: echo.cc
