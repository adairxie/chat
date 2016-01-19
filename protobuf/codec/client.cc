#include "dispatcher.h"
#include "codec.h"
#include "query.pb.h"

#include "../../Logging.h"
#include "../../Mutex.h"
#include "../../EventLoop.h"
#include "../../TcpClient.h"

#include <boost/bind.hpp>

#include <stdio.h>
using namespace muduo;

typedef boost::shared_ptr<Empty> EmptyPtr;
typedef boost::shared_ptr<Answer> AnswerPtr;

google::protobuf::Message* messageToSend;

class QueryClient : boost::noncopyable
{
public:
  QueryClient(EventLoop* loop,
      const InetAddress& serverAddr)
    :loop_(loop),
    client_(loop, serverAddr, "QueryClient"),
    dispatcher_(boost::bind(&QueryClient::onUnknownMessage, this, _1, _2, _3)),
    codec_(boost::bind(&ProtobufDispatcher::onProtobufMessage, &dispatcher_, _1, _2, _3))
  {
    dispatcher_.registerMessageCallback<Answer>(
        boost::bind(&QueryClient::onAnswer, this, _1, _2, _3));
    dispatcher_.registerMessageCallback<Empty>(
        boost::bind(&QueryClient::onEmpty, this, _1, _2, _3));
    client_.setConnectionCallback(
        boost::bind(&QueryClient::onConnection, this, _1));
    client_.setMessageCallback(
        boost::bind(&ProtobufCodec::onMessage, &codec_, _1, _2, _3));
  }

  void connect()
  {
    client_.connect();
  }
private:

  void onConnection(const TcpConnectionPtr& conn)
  {
    LOG_INFO << conn->localAddress().toIpPort() << " -> "
      << conn->peerAddress().toIpPort() << " is "
      << (conn->connected() ? "UP":"DOWN");
    if (conn->connected())
    {
      codec_.send(conn, *messageToSend);
    }
    else
    {
      loop_->quit();
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
  
  EventLoop* loop_;
  TcpClient client_;
  ProtobufDispatcher dispatcher_;
  ProtobufCodec codec_;
};

int main(int argc, char* argv[])
{
  LOG_INFO << "pid = " << getpid();
  if (argc > 2)
  {
    EventLoop loop;
    uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
    InetAddress serverAddr(argv[1], port);

    Query query;
    query.set_id(1);
    query.set_questioner("Xie Hui");
    query.add_question("Runnong?");
    Empty empty;
    messageToSend = &query;

    if (argc > 3 && argv[3][0] == 'e')
    {
      messageToSend = &empty;
    }

    QueryClient client(&loop, serverAddr);
    client.connect();
    loop.loop();
  }
  else
  {
    printf("Usage: %s host_ip port [q|e]\n", argv[0]);
  }
}



