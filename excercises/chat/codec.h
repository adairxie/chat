#ifndef ASIO_CHAT_CODEC_H
#define ASIO_CHAT_CODEC_H

#include "../../Logging.h"
#include "../../Buffer.h"
#include "../../Endian.h"
#include "../../TcpConnection.h"

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>

class LengthHeaderCodec : boost::noncopyable
{
public:
    typedef boost::function<void (const TcpConnectionPtr& conn,
                                  const std::string& message,
                                  Timestamp)> StringMessageCallback;

    explicit LengthHeaderCodec(const StringMessageCallback& cb)
        : messageCallback_(cb)
    {
    }

    void onMessage(const TcpConnectionPtr& conn,
                   Buffer* buf,
                   Timestamp receiveTime)
    {
        while (buf->readableBytes() >= kHeaderLen) 
        {
            const void* data = buf->peek();
            int32_t be32 = *static_cast<const int32_t *>(data);
            const int32_t len = sockets::networkToHost32(be32);
            if (len > 65536 || len < 0)
            {
                LOG_ERROR << "Invalid length " << len;
                conn->shutdown();
                break;
            }
            else if (buf->readableBytes() >= len + kHeaderLen)
            {
                buf->retrieve(kHeaderLen);
                std::string message(buf->peek(), len);
                messageCallback_(conn, message, receiveTime);
                buf->retrieve(len);
            }
            else
            {
                break;
            }
        }
    }
    
    void send(TcpConnection* conn,
              const StringPiece& message)
    {
        Buffer buf;
        buf.append(message.data(), message.size());
        int32_t len = static_cast<int32_t>(message.size());
        int32_t be32 = sockets::hostToNetwork32(len);
        buf.prepend(&be32, sizeof be32);
        conn->send(&buf);
    }

private:
    StringMessageCallback messageCallback_;
    const static size_t kHeaderLen = sizeof(int32_t);
};

#endif
