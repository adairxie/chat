#include "time.h"

#include "../../Logging.h"
#include "../../Endian.h"

#include <boost/bind.hpp>

TimeServer::TimeServer(EventLoop* loop,
                            const InetAddress& listenAddr)
    :server_(loop, listenAddr, "discard")
{
   server_.setConnectionCallback(
           boost::bind(&TimeServer::onConnection, this, _1));
}

void TimeServer::start()
{
    server_.start();
}

void TimeServer::onConnection(const TcpConnectionPtr& conn)
{
    LOG_INFO << "TimeServer - " << conn->localAddress().toIpPort() << " -> "
        << conn->peerAddress().toIpPort() << " is " 
        << (conn->connected() ? "UP":"DOWN");
    if (conn->connected())
    {
        time_t now = ::time(NULL);
        int32_t be32 = sockets::hostToNetwork32(static_cast<int32_t>(now));
        conn->send(&be32, sizeof be32);
        conn->shutdown();
    }
}

