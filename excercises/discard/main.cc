#include "discard.h"
#include "../../EventLoop.h"

int main(void)
{
    EventLoop loop;
    InetAddress listenAddress(9981);
    DiscardServer server(&loop, listenAddress);
    server.start();

    loop.loop();
}
