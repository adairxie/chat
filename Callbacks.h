#ifndef _CALLBACKS_H
#define _CALLBACKS_H

#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

#include "Timestamp.h"

//All client visible callbacks go here

class Buffer;
class TcpConnection;
typedef boost::shared_ptr<TcpConnection> TcpConnectionPtr;

typedef boost::function<void()> TimerCallback;
typedef boost::function<void (const TcpConnectionPtr&)> ConnectionCallback;
typedef boost::function<void (const TcpConnectionPtr&,
		   Buffer* buf,
		   Timestamp)> MessageCallback;

typedef boost::function<void (const TcpConnectionPtr&)> WriteCompleteCallback;
typedef boost::function<void (const TcpConnectionPtr&, size_t)> HighWaterMarkCallback;
typedef boost::function<void (const TcpConnectionPtr&)> CloseCallback;

void defaultConnectionCallback(const TcpConnectionPtr& conn);
void defaultMessageCallback(const TcpConnectionPtr& conn,
                            Buffer* buf,
                            Timestamp receiveTime);
#endif
