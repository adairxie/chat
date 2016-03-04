#include "codec/codec.h"
#include "codec/dispatcher.h"
#include "codec/query.pb.h"

#include "../../Logging.h"
#include "../../Mutex.h"
#include "../../EventLoop.h"
#include "../../TcpClient.h"

#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>

#include <iostream>
#include <stdio.h>

typedef boost::shared_ptr<im::Empty> EmptyPtr;
typedef boost::shared_ptr<im::Answer> AnswerPtr;

google::protobuf::Message* messageToSend;

class ChatClient : boost::noncopyable
{
public: 
   ChatClient(EventLoop* loop, const InetAddress& serverAddr)
       : client_(loop, serverAddr, "ChatClient"),
			 dispatcher_(boost::bind(&ChatClient::onUnknownMessage, this, _1, _2, _3)),
       codec_(boost::bind(&ProtobufDispatcher::onProtobufMessage, &dispatcher_, _1, _2, _3))
    {
				dispatcher_.registerMessageCallback<im::Answer>(
						boost::bind(&ChatClient::onAnswer, this, _1, _2, _3));
				dispatcher_.registerMessageCallback<im::Empty>(
						boost::bind(&ChatClient::onEmpty, this, _1, _2, _3));
        client_.setConnectionCallback(
                boost::bind(&ChatClient::onConnection, this, _1));
        client_.setMessageCallback(
                boost::bind(&ProtobufCodec::onMessage, &codec_, _1, _2, _3));
        client_.enableRetry();
        
    }

   void connect()
   {
        client_.connect();
   }

   void disconnect()
   {
        client_.disconnect();
   }

   void write(const StringPiece& message)
   {
        MutexLockGuard lock(mutex_);
        if (connection_)
        {
       //     codec_.send(get_pointer(connection_), message);
        }
   }

private:
    void onConnection(const TcpConnectionPtr& conn)
    {
        LOG_INFO << conn->localAddress().toIpPort() << " -> "
            << conn->peerAddress().toIpPort() << " is " 
            << (conn->connected() ? "UP" : "DOWN");

        if (conn->connected())
        {
						codec_.send(conn, *messageToSend);
            connection_ = conn;
        }
        else
        {
            connection_.reset();
        }
    }

		void onUnknownMessage(const TcpConnectionPtr& conn,
													const MessagePtr& message,
													Timestamp)
		{
			LOG_INFO << "onUnknownMessage: " << message->GetTypeName();
		}

		void onAnswer(const TcpConnectionPtr&,
				const AnswerPtr& message,
				Timestamp)
		{
			LOG_INFO << "onAnswer:\n" << message->GetTypeName() << message->DebugString();
		}

		void onEmpty(const TcpConnectionPtr&,
								 const EmptyPtr& message,
								 Timestamp)
		{
			LOG_INFO << "onEmpty: " << message->GetTypeName();
		}

    
    TcpClient client_;
		ProtobufDispatcher dispatcher_;
    ProtobufCodec codec_;
    MutexLock mutex_;
    TcpConnectionPtr connection_;
};


int main(int argc, char* argv[])
{
    LOG_INFO << "pid = " << getpid();
    if (argc > 2)
    {
        EventLoop loop;
        uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
        InetAddress serverAddr(argv[1], port);

				im::Query query;
				query.set_id(1);
				query.set_questioner("Xie Hui");
				query.add_question("Is it Running?");
				im::Empty empty;
				messageToSend = &query;

				if (argc > 3 && argv[3][0] == 'e')
				{
					messageToSend = &empty;
				}
        ChatClient client(&loop, serverAddr);
        client.connect();
			  loop.loop();	
        CurrentThread::sleepUsec(1000*1000);
    }
    else
    {
        printf("Usage: %s host_ip port\n", argv[0]);
    }
}
