#ifndef HUB_PUBSUN_H
#define HUB_PUBSUB_H

#include "../../TcpClient.h"
#include "../../Timestamp.h"

namespace pubsub
{
using std::string;

class PubSubClient : boost::noncopyable
{
public:
    typedef boost::function<void (PubSubClient*)> ConnectionCallback;
    typedef boost::function<void (const string& topic,
                            const string& content,
                            Timestamp)> SubscribeCallback;

    PubSubClient(EventLoop* loop,
                const InetAddress& hubAddr,
                const string& name);
    void start();
    void stop();
    bool connected() const;

    void setConnectionCallback(const ConnectionCallback& cb)
    { connectionCallback_ = cb; }

    bool subscribe(const string& topic, const SubscribeCallback& cb);
    void unsubscribe(const string& topic);
    bool publish(const string& topic, const string& content);


private:
    void onConnection(const TcpConnectionPtr& conn);
    void onMessage(const TcpConnectionPtr& conn,
                   Buffer* buf,
                   Timestamp receiveTime);
    bool send(const string& message);

    TcpClient client_;
    TcpConnectionPtr conn_;
    ConnectionCallback connectionCallback_;
    SubscribeCallback subscribeCallback_;
};


}


#endif
