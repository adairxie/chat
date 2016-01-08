#include "discard.h"

#include "../../Logging.h"
#include <boost/bind.hpp>

DiscardServer::DiscardServer(EventLoop* loop,
                            const InetAddress& listenAddr)
    :server_(loop, listenAddr, "discard")
{
   server_.setConnectionCallback(
           boost::bind(&DiscardServer::onConnection, this, _1));
   server_.setMessageCallback(
           boost::bind(&DiscardServer::onMessage, this, _1, _2, _3));
}

void DiscardServer::start()
{
    server_.start();
}

void DiscardServer::onConnection(const TcpConnectionPtr& conn)
{
    LOG_INFO << "DiscardServer - " << conn->localAddress().toIpPort() << " -> "
        << conn->peerAddress().toIpPort() << " is " 
        << (conn->connected() ? "UP":"DOWN");
}

void DiscardServer::onMessage(const TcpConnectionPtr& conn,
                              Buffer* buf,
                              Timestamp time)
{
    std::string msg(buf->retrieveAllAsString());
    LOG_INFO << conn->name() << " discards " << msg.size()
        << " bytes received at " << time.toString();
}
