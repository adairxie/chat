#include "Acceptor.h"

#include "Logging.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "SocketsOps.h"

#include <boost/bind.hpp>
#include <fcntl.h>
#include <errno.h>

using namespace sockets;

Acceptor::Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport)
	:loop_(loop),
	acceptSocket_(sockets::createNonblockingOrDie(AF_INET)),
	acceptChannel_(loop,acceptSocket_.fd()),
	listening_(false),
    idleFd_(::open("/dev/null", O_RDONLY | O_CLOEXEC))
{
    assert(idleFd_ >= 0);
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.setReusePort(reuseport);
    acceptSocket_.bindAddress(listenAddr);
    acceptChannel_.setReadCallback(
		    boost::bind(&Acceptor::handleRead, this));
}

Acceptor::~Acceptor()
{
    acceptChannel_.disableAll();
    acceptChannel_.remove();
    ::close(idleFd_);
}

void Acceptor::listen()
{
   loop_->assertInLoopThread();
   listening_=true;
   acceptSocket_.listen();
   acceptChannel_.enableReading();
}

void Acceptor::handleRead()
{
   loop_->assertInLoopThread();
   InetAddress peerAddr(0);

   int connfd= acceptSocket_.accept(&peerAddr);
   if( connfd >= 0){
       if( newConnectionCallback_)
       {
	      newConnectionCallback_(connfd,peerAddr);
       }
       else 
       {
          sockets::close(connfd);
       }
   }
   else
   {
        LOG_SYSERR << "in Acceptor::handleRead";

        if (errno == EMFILE)
        {
            ::close(idleFd_);
            idleFd_ = ::accept(acceptSocket_.fd(), NULL, NULL);
            ::close(idleFd_);
            idleFd_ = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
        }
   }
}
