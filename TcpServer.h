#ifndef _TCPSERVER_H
#define _TCPSERVER_H

#include "Callbacks.h"
#include "TcpConnection.h"

#include <map>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>


class Acceptor;
class EventLoop;
class EventLoopThreadPool;

class TcpServer: boost::noncopyable
{
  public:
    typedef boost::function<void(EventLoop*)> ThreadInitCallback;
     enum Option
     {
        kNoReusePort,
        kReusePort,
     };

     TcpServer(EventLoop* loop,
               const InetAddress& listenAddr,
               const string& nameArg,
               Option option = kNoReusePort);
     ~TcpServer();  // force out-line dtor, for scoped_ptr members.

     const std::string& ipPort() const { return ipPort_; }
     const std::string& name() const { return name_;}
     EventLoop* getLoop() const {return loop_; }

     ///Starts the server if it's not listening.
     ///
     ///It's harmless to call ut multiple times.
     ///Thread safe.

     //Set the number of threads for handling inpur.
     //
     //Always accepts new connection in loop's thread.
     //Must be called before @c start
     //@param numThreads
     //- 0 means all I/O in loop's thread, no thread will created.
     //- 1 meas all I/O in another thread.
     //- N means a thread pool with N threads, new connections
     //are assigned on a round-robin basis/
     void setThreadNum(int numThreads);
     void setThreadInitCallback(const ThreadInitCallback& cb)
     { threadInitCallback_ = cb; }
     
     boost::shared_ptr<EventLoopThreadPool> threadPool()
     { return threadPool_; }

     ///Starts the server if it's not listening.
     ///
     void start();


     ///Set connection callback.
     /// Not thread safe.
     void setConnectionCallback(const ConnectionCallback& cb )
     { connectionCallback_ = cb; }

     ///Set Message callback
     ///Not thread safe.
     void setMessageCallback(const MessageCallback& cb)
     { messageCallback_ = cb; }

     ///Set write complete callback.
     ///Not thread safe.
     void setWriteCompleteCallback(const WriteCompleteCallback& cb)
     { writeCompleteCallback_ = cb; }
     
  private:
     ///Not thread safe, but in loop.
     void newConnection(int sockfd,const InetAddress& peerAddr);
     ///Thread safe
     void removeConnection(const TcpConnectionPtr& conn);
     ///Not thread safe, but in loop.
     void removeConnectionInLoop(const TcpConnectionPtr& conn);
     typedef std::map<std::string,TcpConnectionPtr> ConnectionMap;
     
     EventLoop* loop_;  // the acceptor loop
     const std::string ipPort_;
     const std::string name_;
     boost::scoped_ptr<Acceptor> acceptor_;  // avoid revealing Acceptor
     boost::shared_ptr<EventLoopThreadPool> threadPool_;
     ConnectionCallback connectionCallback_;
     MessageCallback messageCallback_;
     WriteCompleteCallback writeCompleteCallback_;
     ThreadInitCallback threadInitCallback_;
     bool started_;
     int nextConnId_;  //always in loop thread
     ConnectionMap connections_;
};


#endif
