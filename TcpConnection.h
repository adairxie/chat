#ifndef _TCPCONNECTION_H
#define _TCPCONNECTION_H

#include "Buffer.h"
#include "Callbacks.h"
#include "InetAddress.h"
#include "Logging.h"

#include <boost/any.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/any.hpp>

class Channel;
class EventLoop;
class Socket;

///
///TCP connection,for both client and server usage.
///

class TcpConnection : boost ::noncopyable,
	public boost::enable_shared_from_this<TcpConnection>
{
  public:
      ///Constructs a TcpConnection with a connected sockfd
      ///
      ///User should not create this object.
      TcpConnection(EventLoop* loop,
		      const std::string& name,
		      int sockfd,
		      const InetAddress& localAddr,
		      const InetAddress& peerAddr);
      ~TcpConnection();

      EventLoop* getLoop() const { return loop_;}
      const std::string& name() const { return name_;}
      const InetAddress& localAddress(){ return localAddr_;}
      const InetAddress& peerAddress(){ return  peerAddr_;}
      bool connected() const { return state_ == kConnected;}

      //void send(const void* message,size_t len);
      //Thread safe.
      void send(const void* message, int len);
      void send(const std::string& message);
      void send(Buffer* message);
      //Thread safe.
      void shutdown();
      
      void forceClose();
      
      void setTcpNoDelay(bool on);
      void setKeepAlive(bool on);

      void setConnectionCallback(const ConnectionCallback& cb)
      {  connectionCallback_ =cb; }

      void setMessageCallback(const MessageCallback& cb)
      {  messageCallback_ = cb;}
      
      void setWriteCompleteCallback(const WriteCompleteCallback& cb)
      {  writeCompleteCallback_ = cb; }

      void setHighWaterMarkCallback(const HighWaterMarkCallback& cb, size_t highWaterMark)
      { highWaterMarkCallback_ = cb; highWaterMark_ = highWaterMark; }

      /// Internal use only
      void setCloseCallback(const CloseCallback& cb)
      { closeCallback_ = cb;}
      
      //called when TcpServer accepts a new connection
      void connectEstablished(); // should be called only once
      //called when TcpServer has removed me from its map
      void connectDestroyed();  // should be called only once

      void setContext(const boost::any& context)
      { context_ = context; }

      const boost::any& getContext() const 
      { return context_; }

 private:
      enum StateE { kConnecting,kConnected,kDisconnecting,kDisconnected, };
      void setState(StateE s){ state_ = s;}
      void handleRead(Timestamp receiveTime);
      void handleWrite();
      void handleClose();
      void handleError();
      void sendInLoop(const std::string& message);
      void sendInLoop(const void* message, int len);
      void shutdownInLoop();
      void forceCloseInLoop();


      EventLoop* loop_;
      std::string name_;
      StateE state_;  
      //we don't expose these classes to client.
      boost::scoped_ptr<Socket> socket_;
      boost::scoped_ptr<Channel> channel_;
      InetAddress localAddr_;
      InetAddress peerAddr_;
      ConnectionCallback connectionCallback_;
      MessageCallback    messageCallback_;
      WriteCompleteCallback writeCompleteCallback_;
      HighWaterMarkCallback highWaterMarkCallback_;
      CloseCallback      closeCallback_;

      size_t highWaterMark_;
      Buffer inputBuffer_;
      Buffer outputBuffer_;

      boost::any context_;
};

typedef boost::shared_ptr<TcpConnection> TcpConnectionPtr;

void defaultConnectionCallback(const TcpConnectionPtr& conn);

void defaultMessageCallback(const TcpConnectionPtr& conn,
                            Buffer* buf,
                            Timestamp);

#endif
