#ifndef _ECHO_H
#define _ECHO_H

#include "../../TcpServer.h"

class EchoServer
{
  public:
    EchoServer(EventLoop* loop, 
            const InetAddress& listenAddr);

    void start();

  private:
    void onConnection(const TcpConnectionPtr& conn);

    void onMessage(const TcpConnectionPtr& conn, 
                   Buffer* buf,
                   Timestamp time);

    TcpServer server_;
};

#endif
