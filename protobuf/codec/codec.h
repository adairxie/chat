// references on Shuo Chen.

#ifndef PROTOBUF_CODEC_CODEC_H
#define PROTOBUF_CODEC_CODEC_H

#include "../../Buffer.h"
#include "../../TcpConnection.h"

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

#include <google/protobuf/message.h>

// struct ProtobufTransportFormat __attribute__ ((__packed__))
// {
//   int32_t len;
//   int32_t nameLen;
//   char    typeName[nameLen];
//   char    protobufData[len-nameLen-8];
//   int32_t checkSum; // adler32 of nameLen, typeName and protobufData
// }

typedef boost::shared_ptr<google::protobuf::Message> MessagePtr;

class ProtobufCodec : boost::noncopyable
{
public:

  enum ErrorCode
  {
    kNoError = 0,
    kInvalidLength,
    kCheckSumError,
    kInvalidNameLen,
    kUnkownMessageType,
    kParseError,
  };

  typedef boost::function<void (const TcpConnectionPtr&,
                                const MessagePtr&,
                                Timestamp)> ProtobufMessageCallback;
  typedef boost::function<void (const TcpConnectionPtr&,
                                Buffer*,
                                Timestamp,
                                ErrorCode)> ErrorCallback;
  
  explicit ProtobufCodec(const ProtobufMessageCallback* messageCb)
    : messageCallback_(messageCb),
    errorCallback_(defaultErrorCallback)
  {
  }

  ProbufCodec(const ProbufMessageCallback& messageCb, const ErrorCallback& errorCb)
    : messageCallback_(messageCb),
    errorCallback_(errorCb)
  {
  }

  void onMessage(const TcpConnectionPtr& conn,
                Buffer* buf,
                Timestamp receiveTime);
  
  void send(const TcpConnectionPtr& conn,
            google::protobuf::Message& message)
  {
    Buffer buf;
    fillEmptyBuffer(&buf, message);
    conn->send(&buf);
  }
  
  static const std::string& errorCodeToString(ErrorCode errorCode);
  static void fillEmptyNBuffer(Buffer* buf, const google::protobuf::Message& message);
  static google::protobuf::Message* createMessage(const std::string& type_name);
  static MessagePtr parse(const char* buf, int len, ErrorCode* errorCode);

private:
  static void defaultErrorCallback(const TcpConnectionPtr&,
                                   Buffer*,
                                   Timestamp.
                                   ErrorCode);

  ProtobufMessageCallback messageCallback_;
  ErrorCallback errorCallback_;

  const static int kHeaderLen = sizeof(int32_t);
  const static int kMinMessageLen = 2*kHeaderLen + 2; //nameLen + typeName + ccheckSum
  const static int kMaxMessageLen = 64*1024*1024; 
};

#endif

