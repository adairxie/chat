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
#include <boost/weak_ptr.hpp>
#include <boost/unordered_map.hpp>

#include <set>
#include <stdio.h>
#include <hiredis/hiredis.h>

using namespace im;


typedef boost::shared_ptr<SignUp> SignUpPtr;
typedef boost::shared_ptr<Login>  LoginPtr;
typedef boost::shared_ptr<PMessage> PMessagePtr;
typedef boost::shared_ptr<GMessage> GMessagePtr;
typedef boost::shared_ptr<CreateGroup> CreateGroupPtr;
typedef boost::shared_ptr<AddGroup> AddGroupPtr;
typedef boost::shared_ptr<Success>  SuccessPtr;
typedef boost::shared_ptr<Quit>  QuitPtr;
typedef boost::shared_ptr<MysqlConnection> PMysqlConnectionPtr;

enum UserStaus
{
	Offline, Away, Idle, Online, Busy
};

const char* insertFmt = "INSERT INTO userinfo(user_name, password) VALUES(\'%s\', \'%s\')";
const char* selectFmt = "SELECT password FROM userinfo WHERE user_id = \'%ld\'";
const char* updateFmt = "UPDATE userinfo SET %s = \'%d\' WHERE user_id = \'%ld\'";



class ChatServer : boost::noncopyable
{
    typedef boost::unordered_map<int64_t, TcpConnectionPtr> UserConnectionUMap;
		typedef boost::unordered_multimap<int64_t, TcpConnectionPtr> GroupConnectionUMap;
		typedef GroupConnectionUMap::iterator Giter;
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
				dispatcher_.registerMessageCallback<GMessage>(
						boost::bind(&ChatServer::onGMessage, this, _1, _2, _3));
				dispatcher_.registerMessageCallback<Quit>(
						boost::bind(&ChatServer::onQuit, this, _1, _2, _3));
				dispatcher_.registerMessageCallback<CreateGroup>(
						boost::bind(&ChatServer::onCreateGroup, this, _1, _2, _3));
				dispatcher_.registerMessageCallback<AddGroup>(
						boost::bind(&ChatServer::onAddGroup, this, _1, _2, _3));
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

		int initRedisContext(const char *host, int port, struct timeval timeout)
		{
			redis_ = redisConnectWithTimeout(host, port, timeout);
			if (redis_->err) {
				LOG_INFO << "Connection error: " << redis_->errstr;
				return -1;
			}
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

        if (conn->connected())
        {
            //connections_.insert(conn);
        }
        else
        {
            //connections_.erase(conn);
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
			mysql_->exec("SELECT last_insert_id()");
			const char** str = mysql_->result_row_data(0);
			userconnections_.insert(UserConnectionUMap::value_type(atoi(str[0]), conn));
			lastestRecord_.insert(boost::unordered_map<int64_t, int64_t>::value_type(atoi(str[0]), 0));
      		Success suc;
			suc.set_uid(atoi(str[0]));
			codec_.send(conn,suc);
		}

		void onLogin(const TcpConnectionPtr& conn,
									 const LoginPtr& message, 
									 Timestamp)
		{
			LOG_INFO << "onLogin:\n" << message->GetTypeName() << message->DebugString();
			int64_t uid = message->uid();
			mysql_->exec_format(selectFmt, uid);
		  const  char** pwd = mysql_->result_row_data(0);
			if (pwd[0] == message->passwd()) 
			{
				mysql_->exec_format(updateFmt, "status", Online, uid);
				userconnections_[uid] = conn;
				int lastestIndex = lastestRecord_[uid];
				redisReply* reply = (redisReply*) redisCommand(redis_, "LLEN %ld", uid);
				int currentIndex = reply->integer;
				if (NULL != reply)
				{
					freeReplyObject(reply);
				}
				lastestIndex++;
				if(lastestIndex <= currentIndex) 
				{
					redisReply *reply1 = (redisReply*) redisCommand(redis_, "LRANGE %ld %d %d", uid, lastestIndex, -1);
				LOG_INFO << "uid: " << uid << "lastestIndex: " << lastestIndex << "currentIndx: " << currentIndex;
					if (NULL != reply1)
					{
						PMessage pmessage;
						pmessage.ParseFromArray(reply1->element[0]->str,
								reply1->element[0]->len);
					//	codec_.send(conn, pmessage);

				 //		freeReplyObject(reply);
					}
				}


			}
			
		}

		void onQuit(const TcpConnectionPtr& conn,
								const QuitPtr& message,
								Timestamp)
		{
			LOG_INFO << "onQuit:\n" << message->GetTypeName() << message->DebugString();
      int64_t uid = message->uid();
			mysql_->exec_format(updateFmt, "status", Offline, uid);

			redisReply *reply = (redisReply*) redisCommand(redis_, "LLEN %ld", uid);
			if (NULL != reply)
			{
				lastestRecord_[uid] = reply->integer;
				freeReplyObject(reply);
			}

		}

		void onPMessage(const TcpConnectionPtr& conn,
										const PMessagePtr& message,
										Timestamp)
		{
			LOG_INFO << "onPMessage: " << message->DebugString();
			int64_t peerId = message->peerid();
			boost::weak_ptr<TcpConnection> wconn(userconnections_[peerId]);
			if (!wconn.expired()) 
			{
				boost::shared_ptr<TcpConnection> peercon= wconn.lock();
				codec_.send(peercon, *message);

      	Success suc;
				suc.set_uid(message->uid());
				codec_.send(conn,suc);
			}
			else
			{
				LOG_INFO << "Expired: " << message->DebugString();
//					MemCachedClient mc;
//					mc.Insert("Xie", "Hui");
			}

			const int byteSize = message->ByteSize();
			char buf[byteSize];
			bzero(buf, byteSize);
			message->SerializeToArray(buf, byteSize);
			redisReply* reply = (redisReply*) redisCommand(redis_,
						"RPUSH %ld %b", peerId, buf, byteSize);
			if (NULL != reply)
			{
				freeReplyObject(reply);
			}
		}

		void onGMessage(const TcpConnectionPtr& conn,
										const GMessagePtr& message,
										Timestamp)
		{
			LOG_INFO << "onGMessage: " << message->DebugString();
			for (Giter iter = groupconnections_.begin(); iter != groupconnections_.end(); ++iter) 
			{
				codec_.send(iter->second, *message);
			}
     
		}

		void onCreateGroup(const TcpConnectionPtr& conn,
											 const CreateGroupPtr& message,
											 Timestamp)
		{
			LOG_INFO << "onCreateGroup: " << message->DebugString();
			groupconnections_.insert(GroupConnectionUMap::value_type(message->gid(), conn));
		}

		void onAddGroup(const TcpConnectionPtr& conn,
											 const AddGroupPtr& message,
											 Timestamp)
		{
			LOG_INFO << "onAddGroup: " << message->DebugString();
			groupconnections_.insert(GroupConnectionUMap::value_type(message->gid(), conn));
		}
    TcpServer server_;
		ProtobufDispatcher dispatcher_;
    ProtobufCodec codec_;
    MutexLock mutex_;
		UserConnectionUMap userconnections_;
		GroupConnectionUMap groupconnections_;
		PMysqlConnectionPtr mysql_;
		redisContext* redis_;
    boost::unordered_map<int64_t, int64_t> lastestRecord_;

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
				struct timeval timeout = {1, 500000};
				server.initRedisContext("localhost", 6379, timeout); 
        server.start();
        loop.loop();
    }
    else
    {   
        printf("Usage: %s port [thread_num]\n", argv[0]);
    }
}

