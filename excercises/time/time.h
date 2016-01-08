#ifndef TIME_H
#define TIME_H

#include "../../TcpServer.h"

class TimeServer
{
public:
   TimeServer(EventLoop* loop,
           const InetAddress& listenAddr);

   void start();

private:
   void onConnection(const TcpConnectionPtr& conn);

   TcpServer server_;
};

#endif
