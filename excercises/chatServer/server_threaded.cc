#include "codec/codec.h"
#include "codec/dispatcher.h"
#include "msg.pb.h"
#include "mysql_connection.h"

#include "../../Logging.h"
#include "../../Mutex.h"
#include "../../EventLoop.h"
#include "../../TcpServer.h"

#include <boost/bind.hpp>
#include <boost/make_shared.hpp>

#include <set>
#include <stdio.h>

using namespace im;

typedef boost::shared_ptr<SignUp> SignUpPtr;
typedef boost::shared_ptr<Login>  LoginPtr;
typedef boost::shared_ptr<PMessage> PMessagePtr;
typedef boost::shared_ptr<MysqlConnection> PMysqlConnectionPtr;

const char* insertFmt = "INSERT INTO userinfo(user_name, password) VALUES(\'%s\', \'%s\')";

class ChatServer : boost::noncopyable
{
public:
    ChatServer(EventLoop* loop,
            const InetAddress& listenAddr)
        : server_(loop, listenAddr, "ChatServer"),
				dispatcher_(boost::bind(&ChatServer::onUnknownMessage, this, _1, _2, _3)),
        codec_(boost::bind(&ProtobufDispatcher::onProtobufMessage, &dispatcher_, _1, _2, _3))
    {
				dispatcher_.registerMessageCallback<SignUp>(
						boost::bind(&ChatServer::onSignUp, this, _1, _2, _3));
				dispatcher_.registerMessageCallback<Login>(
						boost::bind(&ChatServer::onLogin, this, _1, _2, _3));
				dispatcher_.registerMessageCallback<PMessage>(
						boost::bind(&ChatServer::onPMessage, this, _1, _2, _3));
        server_.setConnectionCallback(
                boost::bind(&ChatServer::onConnection, this, _1));
        server_.setMessageCallback(
                boost::bind(&ProtobufCodec::onMessage, &codec_, _1, _2, _3));

				mysql_ = boost::make_shared<MysqlConnection>(); 
    }

		int initMysql(const char* host, const char* user, const char* password, const char* db, int port = 3306)
		{
			if (mysql_->init(host, user, password, port) != 0)
				return -1;
			mysql_->use(db);
			return 0;
		}
		

    void setThreadNum(int numThreads)
    {
        server_.setThreadNum(numThreads);
    }

    void start()
    {
        server_.start();
    }

private:
    void onConnection(const TcpConnectionPtr& conn)
    {   
        LOG_INFO << conn->localAddress().toIpPort() << " -> "
            << conn->peerAddress().toIpPort() << " is "
            << (conn->connected() ? "UP":"DOWN");

        MutexLockGuard lock(mutex_);
        if (conn->connected())
        {
            connections_.insert(conn);
        }
        else
        {
            connections_.erase(conn);
        }
    }

		void onUnknownMessage(const TcpConnectionPtr& conn,
													const MessagePtr& message,
													Timestamp)
		{
			LOG_INFO << "onUnkownMessage: " << message->GetTypeName();
			//conn->shutdown();
		}
		
		void onSignUp(const TcpConnectionPtr& conn, 
								 const SignUpPtr& message,
								 Timestamp)
		{
			LOG_INFO << "onSignUp:\n" << message->GetTypeName() << message->DebugString();
			const char* name = message->name().c_str();
			const char* passwd = message->passwd().c_str();
			mysql_->exec_format(insertFmt, name, passwd);
  /*			MutexLockGuard lock(mutex_);
			for (ConnectionList::iterator it = connections_.begin(); it != connections_.end(); ++it)
			{
				codec_.send(*it, answer);
			}
			*/
		}

		void onLogin(const TcpConnectionPtr& conn,
									 const LoginPtr& message, 
									 Timestamp)
		{
			LOG_INFO << "onLogin: " << message->GetTypeName();
		}

		void onPMessage(const TcpConnectionPtr& conn,
										const PMessagePtr& message,
										Timestamp)
		{
			LOG_INFO << "onPMessage: " << message->DebugString();
		}

/*    void onStringMessage(const TcpConnectionPtr& conn,
            const std::string& message,
            Timestamp receiveTime)
    {
        MutexLockGuard lock(mutex_);
        for (ConnectionList::iterator it = connections_.begin(); it != connections_.end(); it++)
        {
            //codec_.send(get_pointer(*it), message);
        }
    }
*/
    typedef std::set<TcpConnectionPtr> ConnectionList;
    TcpServer server_;
		ProtobufDispatcher dispatcher_;
    ProtobufCodec codec_;
    MutexLock mutex_;
    ConnectionList connections_;
		PMysqlConnectionPtr mysql_;
};

int main(int argc, char* argv[])
{
    LOG_INFO << "pid = " << getpid();
    if (argc > 1)
    {
        EventLoop loop;
        uint16_t port = static_cast<uint16_t>(atoi(argv[1]));
        InetAddress serverAddr(port);
        ChatServer server(&loop, serverAddr);
        if (argc > 2)
        {
            server.setThreadNum(atoi(argv[2]));
        }
				server.initMysql("localhost", "root", "242785a", "chat");
        server.start();
        loop.loop();
    }
    else
    {   
        printf("Usage: %s port [thread_num]\n", argv[0]);
    }
}

