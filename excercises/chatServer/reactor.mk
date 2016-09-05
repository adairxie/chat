CXXFLAGS = -O2 -Wall -I  -pthread
LDFLAGS = -lpthread -lz -lmysqlclient -lhiredis

BASE_SRC = Acceptor.cc Buffer.cc Connector.cc Logging.cc LogStream.cc Thread.cc Timestamp.cc Poller.cc TimerQueue.cc Channel.cc TimeZone.cc Exception.cc Date.cc Timer.cc InetAddress.cc Socket.cc SocketsOps.cc  TcpConnection.cc TcpServer.cc EventLoopThread.cc EventLoopThreadPool.cc

#BASE_HEADERS = $(addprefix ../../, $(patsubst %.cc, %.h, $(BASE_SRC)))
$(BINARIES):
	g++ $(CXXFLAGS) -o $@ $(LIB_SRC)  $(addprefix ../../, $(BASE_SRC)) $(filter %.cc,$^) $(LDFLAGS) `pkg-config --cflags --libs protobuf`

clean:
	rm -f $(BINARIES) core

