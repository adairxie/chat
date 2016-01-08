#include "time.h"
#include "../../EventLoop.h"

int main(void)
{
    EventLoop loop;
    InetAddress listenAddress(9981);
    TimeServer server(&loop, listenAddress);
    server.start();

    loop.loop();
}
