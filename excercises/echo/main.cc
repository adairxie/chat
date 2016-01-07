#include "echo.h"

#include "../../EventLoop.h"

int main()
{
    EventLoop loop;
    InetAddress listenAddr(2007);
    EchoServer server(&loop, listenAddr);
    server.start();
    loop.loop();
    return 0;
}
