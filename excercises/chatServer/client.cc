#include "codec/codec.h"
#include "codec/dispatcher.h"
#include "msg.pb.h"
#include "user.h"

#include "../../Logging.h"
#include "../../Mutex.h"
#include "../../EventLoopThread.h"
#include "../../TcpClient.h"

#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>

#include <iostream>
#include <fstream>
#include <vector>
using namespace im; //message namespace 

typedef boost::shared_ptr<Empty> EmptyPtr;
typedef boost::shared_ptr<PMessage> PMessagePtr;
typedef boost::shared_ptr<GMessage> GMessagePtr;
typedef boost::shared_ptr<Success> SuccessPtr;


volatile google::protobuf::Message* messageToSend;
volatile google::protobuf::Message* messageReceived;


std::vector<std::string> split(std::string src, char split)
{
	std::vector<std::string> res;
	std::string tmp;
	size_t i;
	int count = 0;
	for (i=0; i <src.size(); ++i)
		if (src[i] == split)
			count++;
	for (i=0; i < src.size() && count >0 ; ++i) {
		if (src[i] != split) {
			tmp.insert(tmp.size(),1,src[i]);
		}
		else
		{
			if (tmp.size())
				res.push_back(tmp);
			tmp.clear();
			count--;
		}
	}
	res.push_back(string(src, i));
	return res;
}

class ChatClient : boost::noncopyable
{
public: 
   ChatClient(EventLoop* loop, const InetAddress& serverAddr)
       : client_(loop, serverAddr, "ChatClient"),
			 dispatcher_(boost::bind(&ChatClient::onUnknownMessage, this, _1, _2, _3)),
       codec_(boost::bind(&ProtobufDispatcher::onProtobufMessage, &dispatcher_, _1, _2, _3))
    {
				dispatcher_.registerMessageCallback<PMessage>(
						boost::bind(&ChatClient::onPrivateChat, this, _1, _2, _3));
				dispatcher_.registerMessageCallback<GMessage>(
						boost::bind(&ChatClient::onGroupChat, this, _1, _2, _3));
				dispatcher_.registerMessageCallback<Empty>(
						boost::bind(&ChatClient::onEmpty, this, _1, _2, _3));
				dispatcher_.registerMessageCallback<Success>(
						boost::bind(&ChatClient::onSuccess, this, _1, _2, _3));
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

   void write(const google::protobuf::Message& message)
	 {
        MutexLockGuard lock(mutex_);
        while(!connection_)
					;
				codec_.send(connection_, message);
   }

private:
    void onConnection(const TcpConnectionPtr& conn)
    {
        LOG_INFO << conn->localAddress().toIpPort() << " -> "
            << conn->peerAddress().toIpPort() << " is " 
            << (conn->connected() ? "UP" : "DOWN");

        if (conn->connected())
        {
						//codec_.send(conn, *messageToSend);
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

		void onPrivateChat(const TcpConnectionPtr&,
				const PMessagePtr& message,
				Timestamp)
		{
			LOG_INFO << "onPMessage:\n" << message->GetTypeName() << message->DebugString();
		}

		void onGroupChat(const TcpConnectionPtr&,
				const GMessagePtr& message,
				Timestamp)
		{
			LOG_INFO << "onGMessage:\n" << message->GetTypeName() << message->DebugString();
		}

		void onSuccess(const TcpConnectionPtr&,
									 const SuccessPtr& message,
									 Timestamp)
		{
			LOG_INFO << "onSuccess:\n" << message->GetTypeName() <<" " <<message->DebugString();
			messageReceived = new Success(*message);
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

void dispatch(ChatClient& client, vector<string>& message);

int main(int argc, char* argv[])
{
    LOG_INFO << "pid = " << getpid();
    if (argc > 2)
    {
        EventLoopThread loopThread;
        uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
        InetAddress serverAddr(argv[1], port);

        ChatClient client(loopThread.startLoop(), serverAddr);
        client.connect();

				std::string in;	
				std::vector<std::string> parseResult;
				while (true)
				{
					getline(std::cin, in);
					parseResult = split(in,' ');
					if (parseResult.size() > 0)
					  dispatch(client, parseResult);
				}
    }
    else
    {
        printf("Usage: %s host_ip port\n", argv[0]);
    }
}



void sign(ChatClient& client, vector<string>& message)
{
		ofstream out("user.info", ofstream::out);
		SignUp signUp;
		signUp.set_name(message[1]);
		signUp.set_passwd(message[2]);
		client.write(signUp);
	  	while (messageReceived == NULL);	
		int64_t id = (*(Success*)messageReceived).uid();
		out << id << endl;
		out << message[1] << endl;
		out << message[2] << endl;
		delete messageReceived;
}
void login(ChatClient& client, vector<string>& message)
{
		Login lmsg;
		lmsg.set_uid(atol(message[1].c_str()));
		lmsg.set_passwd(message[2]);
		client.write(lmsg);
}

void onPrivateChat(ChatClient& client, vector<string>& message)
{
		ifstream in("user.info", ifstream::in);
		string uid;
		getline(in, uid);
		PMessage pmessage;
		pmessage.set_uid(atoi(uid.c_str()));
		pmessage.set_peerid(atol(message[1].c_str()));
		string content;
		for (size_t i = 2; i < message.size(); ++i)
				content += message[i] + " ";
		pmessage.set_content(content);
		client.write(pmessage);
}

void quit(ChatClient& client)
{
		ifstream in("user.info", ifstream::in);
		string uid;
		getline(in, uid);
		Quit message;
		message.set_uid(atol(uid.c_str()));
		client.write(message);
		client.disconnect();
    	//CurrentThread::sleepUsec(1000*1000);
}

void groupChat(ChatClient& client, vector<string>& message)
{
		GMessage gmessage;
		gmessage.set_gid(atol(message[1].c_str()));
		string gmsg;
		for (size_t i = 1; i < message.size(); ++i)
				gmsg += message[i] + " ";
		gmessage.set_content(gmsg);
		client.write(gmessage);
}

void createGroup(ChatClient& client, vector<string>& message)
{
	CreateGroup group;
	group.set_gid(atol(message[1].c_str()));
	client.write(group);
}

void addGroup(ChatClient& client, vector<string>& message)
{
	AddGroup group;
	group.set_gid(atol(message[1].c_str()));
	client.write(group);
}

void dispatch(ChatClient& client, vector<string>& message)
{
	switch(message[0][0]) {
		case 's':
				sign(client, message);
				break;
		case 'l':
				login(client, message);
				break;
		case 'p':
				onPrivateChat(client, message);
				break;
		case 'c':
				createGroup(client, message);
				break;
		case 'a':
				addGroup(client, message);
				break;
		case 'g':
				groupChat(client, message);
			break;
		case 'q':
				quit(client);
				break;
		default:
			std::cout << "Wrong Type of Message!" << std::endl;
	};
}
