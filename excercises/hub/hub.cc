#include "codec.h"
#include "../../Logging.h"
#include "../../EventLoop.h"
#include "../../TcpServer.h"

#include <boost/bind.hpp>

#include <map>
#include <set>
#include <stdio.h>

namespace pubsub
{
typedef std::set<string> ConnectionSubscription;

class Topic 
{
 public:
   Topic(const string& topic)
     : topic_(topic)
   {
   }

   void add(const TcpConnectionPtr& conn)
   {
     audiences_.insert(conn);
     if (lastPubTime_.valid())
     {
       conn->send(makeMessage());
     }
   }

   void remove(const TcpConnectionPtr& conn)
   {
     audiences_.erase(conn);
   }

   void publish(const string& content, Timestamp time)
   {
     content_ = content;
     lastPubTime_ = time;
     string message = makeMessage();
     for (std::set<TcpConnectionPtr>::iterator it = audiences_.begin();
         it != audiences_.end(); ++it)
     {
       (*it)->send(message);
     }
   }

 private:
   
   string makeMessage()
   {
    return "pub " + topic_ + "\r\n" + content_ + "\r\n";
   }

   string topic_;
   string content_;
   Timestamp lastPubTime_;
   std::set<TcpConnectionPtr> audiences_;
};

class PubSubServer : boost::noncopyable
{
 public:
  PubSubServer(EventLoop* loop, const InetAddress& listenAddr)
    : server_(loop, listenAddr, "PubSubSerber")
  {
    server_.setConnectionCallback(
        boost::bind(&PubSubServer::onConnecton, this, _1));
    server_.setMessageCallback(
        boost::bind(&PubSubServer::onMessage, this, _1, _2, _3));
//    loop_->runEvery(1.0, boost::bind(&PubSubServer::timePublish, this));
  }

  void start()
  {
    server_.start();
  }

 private:
  void onConnecton(const TcpConnectionPtr& conn)
  {
    if (conn->connected())
    {
      conn->setContext(ConnectionSubscription());
    }
    else
    {
      const ConnectionSubscription& connSub
        = boost::any_cast<const ConnectionSubscription&>(conn->getContext());

      for (ConnectionSubscription::const_iterator it = connSub.begin();
          it != connSub.end();)
      {
        doUnsubscribe(conn, *it++);
      }

    }
  }

  void onMessage(const TcpConnectionPtr& conn,
                Buffer* buf,
                Timestamp receiveTime)
  {
    ParseResult result = kSuccess;
    while (result == kSuccess)
    {
      string cmd;
      string topic;
      string content;
      result = parseMessage(buf, &cmd, &topic, &content);
      if (result == kSuccess)
      {
        if (cmd == "pub")
        {
          doPublish(conn->name(), topic, content, receiveTime);
        }
        else if (cmd == "sub")
        {
          LOG_INFO << conn->name() << " subscribes " << topic;
          doSubscribe(conn, topic);
        }
        else if (cmd == "unsub")
        {
          doUnsubscribe(conn, topic);
        }
        else
        { 
         conn->shutdown();
         result = kError;
        }
      }
      else if (result == kError)
      {
        conn->shutdown();
      }
    }
  }

  void timePublish()
  {
   Timestamp now = Timestamp::now();
   doPublish("internal", "utc_time", now.toFormattedString(), now);
  }

  void doSubscribe(const TcpConnectionPtr& conn,
                   const string& topic)
  {
    ConnectionSubscription* connSub
      = boost::any_cast<ConnectionSubscription>(conn->getMutableContext());

    connSub->insert(topic);
    getTopic(topic).add(conn);
  }

  void doUnsubscribe(const TcpConnectionPtr& conn,
                     const string& topic)
  {
    LOG_INFO << conn->name() << " unsubscribes " << topic;
    getTopic(topic).remove(conn);

    ConnectionSubscription* connSub
      = boost::any_cast<ConnectionSubscription>(conn->getMutableContext());
    connSub->erase(topic);
  }

  void doPublish(const string& source,
                 const string& topic,
                 const string& content,
                 Timestamp time)
  {
    getTopic(topic).publish(content, time);
  }
  Topic& getTopic(const string& topic)
  {
    std::map<string, Topic>::iterator it = topics_.find(topic);
    if (it == topics_.end())
    {
      it = topics_.insert(std::make_pair(topic, Topic(topic))).first;
    }
    return it->second;
  }
  EventLoop* loop_;
  TcpServer server_;
  std::map<string, Topic> topics_;
};

}

int main(int argc, char* argv[])
{
  if (argc > 1)
  {
    uint16_t port = static_cast<uint16_t>(atoi(argv[1]));
    EventLoop loop;
    if (argc > 2)
    {

    }
    pubsub::PubSubServer server(&loop, InetAddress(port));
    server.start();
    loop.loop();
  }
  else
  {
    printf("Usage: %s pubsub_port [inspect_port]\n", argv[0]);
  }
}

