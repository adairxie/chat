#include "TcpServer.h"

#include "Logging.h"
#include "Acceptor.h"
#include "EventLoop.h"
#include "SocketsOps.h"
#include "EventLoopThreadPool.h"
#include <boost/bind.hpp>
#include <stdio.h>

TcpServer::TcpServer(EventLoop* loop, 
                    const InetAddress& listenAddr,
                    const std::string& nameArg,
                    Option option)
	:loop_(loop),
	ipPort_(listenAddr.toIpPort()),
  name_(nameArg),
	acceptor_(new Acceptor(loop, listenAddr, option == kReusePort)),
	threadPool_(new EventLoopThreadPool(loop, name_)),
  connectionCallback_(defaultConnectionCallback),
  messageCallback_(defaultMessageCallback),
	started_(false),
	nextConnId_(1)
{
    acceptor_->setNewConnectionCallback(
        boost::bind(&TcpServer::newConnection,this,_1,_2));
   
}

TcpServer::~TcpServer()
{

}

void TcpServer::setThreadNum(int numThreads)
{
  assert(0 <= numThreads);
  threadPool_->setThreadNum(numThreads);
}

void TcpServer::start()
{
  if(!started_)
  {
    started_ = true;
		threadPool_->start();
  	if(!acceptor_->listening())
  	{
    	loop_->runInLoop(
        boost::bind(&Acceptor::listen, get_pointer(acceptor_)));
  	} 
	}
}

void TcpServer::newConnection(int sockfd,const InetAddress& peerAddr)
{
   loop_->assertInLoopThread();
   char buf[32];
   snprintf(buf,sizeof buf, "#%d",nextConnId_);
   ++nextConnId_;
   std::string connName = name_ + buf;

   InetAddress localAddr(sockets::getLocalAddr(sockfd));
   //Poll with zedro timeout to doule confirm the new connection
   EventLoop* ioLoop =threadPool_->getNextLoop();
   LOG_INFO <<"tid "<<ioLoop->getThreadId()<<" " <<"TcpServer::newConnection ["<< name_ <<
	   "] - new connection [" << connName << "] from " <<peerAddr.toIpPort();
   TcpConnectionPtr conn(
     new TcpConnection(ioLoop,connName,sockfd,localAddr,peerAddr));
   connections_[connName]=conn;
   conn->setConnectionCallback(connectionCallback_);
   conn->setMessageCallback(messageCallback_);
   conn->setWriteCompleteCallback(writeCompleteCallback_);
   conn->setCloseCallback(
      boost::bind(&TcpServer::removeConnection,this,_1));
   ioLoop->runInLoop(boost::bind(&TcpConnection::connectEstablished,conn));
}


void TcpServer::removeConnection(const TcpConnectionPtr& conn)
{
  loop_->runInLoop(
     boost::bind(&TcpServer::removeConnectionInLoop,this,conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn)
{
   loop_->assertInLoopThread();
   LOG_INFO << "TcpServer::removeConnectionInLoop ["<< name_
	   << "] - connection " << conn->name();
   size_t n =connections_.erase(conn->name());
   assert( n== 1 ); (void)n;
   EventLoop* ioLoop=conn->getLoop();
   ioLoop->queueInLoop(
	boost::bind(&TcpConnection::connectDestroyed,conn));
}


