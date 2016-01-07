#ifndef __ACCEPTOR_H
#define __ACCEPTOR_H

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>

#include "Channel.h"
#include "Socket.h"

class EventLoop;
class InetAddress;

///
///Acceptors of incoming connections.
///

class Acceptor: boost::noncopyable
{
  public:
    typedef boost::function<void(int sockfd,
		    const InetAddress&)> NewConnectionCallback;

    Acceptor(EventLoop* loop,const InetAddress& listenAddr, bool reuseport);
    ~Acceptor();

    void setNewConnectionCallback(const NewConnectionCallback& cb)
    { newConnectionCallback_=cb;}

    bool listening() const { return listening_;}
    void listen();

  private:
    void handleRead();

    EventLoop* loop_;
    Socket acceptSocket_;
    Channel acceptChannel_;
    NewConnectionCallback newConnectionCallback_;
    bool listening_;
    int idleFd_;
};



#endif


