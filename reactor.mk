CXXFLAGS = -O0 -g  -Wall -I  -pthread
LDFLAGS = -lpthread
BASE_SRC = Acceptor.cc Buffer.cc Connector.cc Logging.cc LogStream.cc Thread.cc Timestamp.cc Poller.cc TimerQueue.cc Channel.cc TimeZone.cc Exception.cc Date.cc Timer.cc InetAddress.cc Socket.cc SocketsOps.cc  TcpConnection.cc TcpServer.cc EventLoopThread.cc EventLoopThreadPool.cc

$(BINARIES):
	g++ $(CXXFLAGS) -o $@ $(LIB_SRC) $(BASE_SRC) $(filter %.cc,$^) $(LDFLAGS)

clean:
	rm -f $(BINARIES) core

