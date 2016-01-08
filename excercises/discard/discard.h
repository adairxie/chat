#ifndef DISCARD_H
#define DISCARD_H

#include "../../TcpServer.h"

class DiscardServer
{
public:
   DiscardServer(EventLoop* loop,
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
