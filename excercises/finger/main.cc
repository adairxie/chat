#include "../../EventLoop.h"
#include "../../TcpServer.h"

typedef std::map<std::string, std::string> UserMap;
UserMap users;

std::string getUser(const std::string& user)
{
    std::string result = "No such user";
    UserMap::iterator it = users.find(user);
    if (it != users.end())
    {
        result = it->second;
    }
    return result;
}


void onConnection(const TcpConnectionPtr& conn)
{
  if (conn->connected())
  {
      conn->send("hello World\n");
  }
}

void onMessage(const TcpConnectionPtr& conn,
        Buffer* buf,
        Timestamp receiveTime)
{
    const char* crlf = buf->findCRLF();
    if (crlf)
    {
        std::string user(buf->peek(), crlf);
        conn->send(getUser(user) + "\r\n");
        buf->retrieveUntil(crlf+2);
        conn->shutdown();
    }
}

int main()
{
    users["xiehui"] = "Happy and Well";
    EventLoop loop;
    TcpServer server(&loop, InetAddress(1079), "Finger");
    server.setConnectionCallback(onConnection);
    server.setMessageCallback(onMessage);
    server.start();
    loop.loop();

    return 0;
}
