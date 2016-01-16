#include "codec.h"

#include "../../Logging.h"
#include "../../Mutex.h"
#include "../../EventLoop.h"
#include "../../TcpServer.h"

#include <boost/bind.hpp>

#include <set>
#include <stdio.h>

class ChatServer : boost::noncopyable
{
public:
    ChatServer(EventLoop* loop,
            const InetAddress& listenAddr)
        : server_(loop, listenAddr, "ChatServer"),
        codec_(boost::bind(&ChatServer::onStringMessage, this, _1, _2, _3))
    {
        server_.setConnectionCallback(
                boost::bind(&ChatServer::onConnection, this, _1));
        server_.setMessageCallback(
                boost::bind(&LengthHeaderCodec::onMessage, &codec_, _1, _2, _3));
    }

    void start()
    {
        server_.start();
    }

private:
    void onConnection(const TcpConnectionPtr& conn)
    {
        LOG_INFO << conn->localAddress().toIpPort() << " -> "
            << conn->peerAddress().toIpPort() << " is "
            << (conn->connected() ? "UP" : "DOWN");

        if (conn->connected())
        {
            connections_.insert(conn);
        }
        else
        {
            connections_.erase(conn);
        }
    }

    void onStringMessage(const TcpConnectionPtr&,
                         const std::string& message,
                         Timestamp)
    {
        for (ConnectionList::iterator it = connections_.begin();
                it != connections_.end();
                ++it)
        {
            codec_.send(get_pointer(*it), message);
        }
    }

    typedef std::set<TcpConnectionPtr> ConnectionList;
    TcpServer server_;
    LengthHeaderCodec codec_;
    ConnectionList connections_;

};


int main(int argc, char *argv[])
{
    LOG_INFO << "pid = " << getpid();
    if (argc > 1)
    {
        EventLoop loop;
        uint16_t port = static_cast<uint16_t>(atoi(argv[1]));
        InetAddress serverAddr(port);
        ChatServer server(&loop, serverAddr);
        server.start();
        loop.loop();
    }
    else
    {
        printf("Usage: %s port\n", argv[0]);
    }
}
