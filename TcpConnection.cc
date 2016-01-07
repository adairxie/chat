#include "TcpConnection.h"

#include "Channel.h"
#include "EventLoop.h"
#include "Socket.h"
#include "SocketsOps.h"

#include <boost/bind.hpp>

#include <errno.h>
#include <stdio.h>

using namespace sockets;

void defaultConnectionCallback(const TcpConnectionPtr& conn)
{
    LOG_TRACE << conn->localAddress().toIpPort() << " -> "
        << conn->peerAddress().toIpPort() << " is "
        << (conn->connected() ? "UP" : "DOWN");
}

void defaultMessageCallback(const TcpConnectionPtr& conn,
                            Buffer* buf,
                            Timestamp)
{
    buf->retrieveAll();
}

TcpConnection::TcpConnection(EventLoop* loop,
	const std::string& nameArg,
	int sockfd,
	const InetAddress& localAddr,
	const InetAddress& peerAddr)
   :loop_(loop),
    name_(nameArg),
    state_(kConnecting),
    socket_(new Socket(sockfd)),
    channel_(new Channel(loop,sockfd)),
    localAddr_(localAddr),
    peerAddr_(peerAddr),
    highWaterMark_(64*1024*1024)
{
   LOG_DEBUG << "TcpConnection::ctor[" << name_ <<"]" << this
	   << " fd=" << sockfd;
   socket_->setKeepAlive(true);

   channel_->setReadCallback(
		   boost::bind(&TcpConnection::handleRead,this,_1));
   channel_->setWriteCallback(
		   boost::bind(&TcpConnection::handleWrite,this));
   channel_->setCloseCallback(
		   boost::bind(&TcpConnection::handleClose,this));
   channel_->setErrorCallback(
		   boost::bind(&TcpConnection::handleError,this));
}

TcpConnection::~TcpConnection()
{
   LOG_DEBUG << "TcpConnection::dtor[" << name_ <<" ]at" << this
	   << " fd=" << channel_->fd();
}

void TcpConnection::send(const void* data, int len)
{
    send(std::string(static_cast<const char*>(data), len));
}

void TcpConnection::send(const std::string& message)
{
   if(state_ == kConnected ){
     if(loop_->isInLoopThread()){
	sendInLoop(message);
     }else{
       loop_->runInLoop(
          boost::bind(&TcpConnection::sendInLoop,this,message));
     }
   }
}

void TcpConnection::send(Buffer* buf)
{
    if (state_ == kConnected)
    {
      if (loop_->isInLoopThread())
      {
        sendInLoop(buf->peek(), buf->readableBytes());
        buf->retrieveAll();
      }
      else
      {
        loop_->runInLoop(
                boost::bind(&TcpConnection::sendInLoop,
                    this,
                    buf->retrieveAllAsString()));
      }
    }
}
void TcpConnection::sendInLoop(const void* message, int len)
{
    sendInLoop(std::string(static_cast<const char*>(message), len));
}

void TcpConnection::sendInLoop(const std::string& message)
{
   loop_->assertInLoopThread();
   ssize_t nwrote =0;
   size_t len=message.size();
   size_t remaining = len;
   bool faultError = false;
   if (state_ == kDisconnected)
   {
      LOG_WARN << "disconnected, give up writting";
      return;
   }
   //if no thing in output queue,try writting directly.
   if( !channel_->isWriting() && outputBuffer_.readableBytes() == 0){
     nwrote= ::write(channel_->fd(),message.data(),message.size());
     if(nwrote >= 0){
	     remaining = len- nwrote;
         if( implicit_cast<size_t>(nwrote) < message.size()){
	     LOG_TRACE << "I am going to write more data";
         } else if(remaining == 0 && writeCompleteCallback_){
	     loop_->queueInLoop(
	        boost::bind(writeCompleteCallback_,shared_from_this()));
	}
     }else{
       nwrote = 0;
       if( errno != EWOULDBLOCK) 
       {
 	       LOG_SYSERR << "TcpConnection::sendInLoop";
         if (errno == EPIPE || errno == ECONNRESET) 
         {
            faultError = true;
         }
       }
    }
   }

   assert(remaining <= len); // >0?
   if (!faultError && remaining > 0)
   {
     size_t oldLen = outputBuffer_.readableBytes();
     if (oldLen + remaining >= highWaterMark_
         && oldLen < highWaterMark_
         && highWaterMarkCallback_)
     {
       loop_->queueInLoop(
           boost::bind(highWaterMarkCallback_, shared_from_this(), oldLen + remaining));
     }
     outputBuffer_.append(static_cast<const char*>(message.data()) + nwrote, remaining);
	   if( !channel_->isWriting()){
              channel_->enableWriting();
	   }
   }
   
}

void TcpConnection::shutdown()
{
   //FIXME: use compare and swap
   if(state_ == kConnected)
   {
      setState(kDisconnecting);
      //FIXME: shared_from_this()?
      loop_->runInLoop(
		boost::bind(&TcpConnection::shutdownInLoop,this));
   }
}

void TcpConnection::shutdownInLoop()
{
   loop_->assertInLoopThread();
   if(!channel_->isWriting())
   {
      //we are not writing
      socket_->shutdownWrite();
   }
}

void TcpConnection::forceClose()
{
    if (state_ == kConnected || state_ == kDisconnecting)
    {
        setState(kDisconnecting);
        loop_->queueInLoop(boost::bind(&TcpConnection::forceCloseInLoop, shared_from_this()));
    }
}

void TcpConnection::forceCloseInLoop()
{
    loop_->assertInLoopThread();
    if (state_ == kConnected || state_ == kDisconnecting)
    {
        handleClose();
    }
}

void TcpConnection::setTcpNoDelay(bool on)
{
   socket_->setTcpNoDelay(on);
}

void TcpConnection::setKeepAlive(bool on)
{
   socket_->setKeepAlive(on);
}

void TcpConnection::connectEstablished()
{
   loop_->assertInLoopThread();
   assert(state_ == kConnecting);
   setState(kConnected);
   channel_->enableReading();

   connectionCallback_(shared_from_this());
}
void TcpConnection::connectDestroyed()
{
  loop_->assertInLoopThread();
  assert(state_ == kConnected || state_ == kDisconnecting);
  setState(kDisconnected);
  channel_->disableAll();
  connectionCallback_(shared_from_this());

  loop_->removeChannel(get_pointer(channel_));
}


void TcpConnection::handleRead(Timestamp receiveTime)
{
   int savedErrno =0;
   ssize_t n = inputBuffer_.readFd(channel_->fd(),&savedErrno);
   if ( n>0 ){
         messageCallback_(shared_from_this(),&inputBuffer_,receiveTime);
   }else if ( n == 0 ){
	 handleClose();
   }else{
	 errno=savedErrno;
	 LOG_SYSERR << "TcpConnection::handleRead";
	 handleError();
   }
   //FIXME close conneciton if n==0
}
void TcpConnection::handleWrite()
{
   loop_->assertInLoopThread();
   if( channel_->isWriting()){
	ssize_t n= ::write(channel_->fd(),
			   outputBuffer_.peek(),
			   outputBuffer_.readableBytes());
	if( n > 0){
	    outputBuffer_.retrieve(n);
	    if(outputBuffer_.readableBytes() == 0 ) {
		channel_->disableWriting();
		if(writeCompleteCallback_){
		    loop_->queueInLoop(
		        boost::bind(writeCompleteCallback_,shared_from_this()));
		}
		if(state_ == kDisconnecting){
	          shutdownInLoop();
		}
	   }else{
	      LOG_TRACE << "I am going to write more data";
	   }
	}else {
	   LOG_SYSERR << "TcpConnection::handleWrite";
	}
   } else {
      LOG_TRACE << "Connection is down, no more writing";
   }
}

void TcpConnection::handleClose()
{
   loop_->assertInLoopThread();
   LOG_TRACE << "TcpConnection:L:handleClose state = " <<state_;
   assert(state_ == kConnected || state_ == kDisconnecting);
   //we don't close fd,leave it to dtor, so we can find leaks easily.
   channel_->disableAll();
   //must be the last line
   closeCallback_(shared_from_this());
}

void TcpConnection::handleError()
{
   int err=sockets::getSocketError(channel_->fd());
   LOG_ERROR << "TcpConnection::handleError [" << name_
	   <<"] - SO_ERROR = " << err <<" " <<strerror_tl(err);
}





