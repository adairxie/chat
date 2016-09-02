#VPATH = /home/chat/
LIB_SRC = ../../EventLoop.cc 
BINARIES = server_threaded chatClient stress

all: $(BINARIES)

include reactor.mk

#chatServer: server.cc 
<<<<<<< HEAD
chatClient: client.cc ../../TcpClient.cc msg.pb.cc codec/codec.cc
sign_up: sign_up.cc ../../TcpClient.cc msg.pb.cc codec/codec.cc
#stress_test: stress_test.cc msg.pb.cc codec/codec.cc 
stress: stress.cc ../../TcpClient.cc msg.pb.cc codec/codec.cc
#loadtest: loadtest.cc ../../TcpClient.cc

server_threaded: server_threaded.cc msg.pb.cc mysql_connection.cc codec/codec.cc
=======
chatClient: client.cc ../../TcpClient.cc codec/codec.cc msg.pb.cc user.cc
#loadtest: loadtest.cc ../../TcpClient.cc

server_threaded: server_threaded.cc codec/codec.cc msg.pb.cc mysql_connection.cc
>>>>>>> 18a97030770e436606799ba9bbc4e5a16234dd86
#server_threaded_efficient: server_threaded_efficient.cc
#server_threaded_highperformance: server_threaded_highperformance.cc


install:
	mv  sign_up test/

.PHONY: clean
clean:
<<<<<<< HEAD
	rm -rf loadtest server_threaded_highperformance server_threaded chatClient server_threaded_efficient
=======
	-rm -rf server_threaded chatClient
>>>>>>> 18a97030770e436606799ba9bbc4e5a16234dd86
