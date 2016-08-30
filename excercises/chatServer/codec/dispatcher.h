// Copyright 2011, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef MUDUO_EXAMPLES_PROTOBUF_CODEC_DISPATCHER_H
#define MUDUO_EXAMPLES_PROTOBUF_CODEC_DISPATCHER_H

#include "../../../Callbacks.h"

#include <google/protobuf/message.h>

#include <map>

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

#ifndef NDEBUG
#include <boost/static_assert.hpp>
#include <boost/type_traits/is_base_of.hpp>
#endif

typedef boost::shared_ptr<google::protobuf::Message> MessagePtr;

class Callback : boost::noncopyable
{
 public:
  virtual ~Callback() {};
  virtual void onMessage(const TcpConnectionPtr&,
                         const MessagePtr& message,
                         Timestamp) const = 0;
};

template <typename T>
class CallbackT : public Callback
{
#ifndef NDEBUG
  BOOST_STATIC_ASSERT((boost::is_base_of<google::protobuf::Message, T>::value));
#endif
 public:
  typedef boost::function<void (const TcpConnectionPtr&,
                                const boost::shared_ptr<T>& message,
                                Timestamp)> ProtobufMessageTCallback;

  CallbackT(const ProtobufMessageTCallback& callback)
    : callback_(callback)
  {
  }

  virtual void onMessage(const TcpConnectionPtr& conn,
                         const MessagePtr& message,
                         Timestamp receiveTime) const
  {
    boost::shared_ptr<T> concrete = down_pointer_cast<T>(message);
    assert(concrete != NULL);
    callback_(conn, concrete, receiveTime);
  }

 private:
  ProtobufMessageTCallback callback_;
};

class ProtobufDispatcher
{
 public:
  typedef boost::function<void (const TcpConnectionPtr&,
                                const MessagePtr& message,
                                Timestamp)> ProtobufMessageCallback;

  explicit ProtobufDispatcher(const ProtobufMessageCallback& defaultCb)
    : defaultCallback_(defaultCb)
  {
  }

  void onProtobufMessage(const TcpConnectionPtr& conn,
                         const MessagePtr& message,
                         Timestamp receiveTime) const
  {
    CallbackMap::const_iterator it = callbacks_.find(message->GetDescriptor());
    if (it != callbacks_.end())
    {
      it->second->onMessage(conn, message, receiveTime);
    }
    else
    {
      defaultCallback_(conn, message, receiveTime);
    }
  }

  template<typename T>
  void registerMessageCallback(const typename CallbackT<T>::ProtobufMessageTCallback& callback)
  {
    boost::shared_ptr<CallbackT<T> > pd(new CallbackT<T>(callback));
    callbacks_[T::descriptor()] = pd;
  }

 private:
  typedef std::map<const google::protobuf::Descriptor*, boost::shared_ptr<Callback> > CallbackMap;

  CallbackMap callbacks_;
  ProtobufMessageCallback defaultCallback_;
};
#endif  // MUDUO_EXAMPLES_PROTOBUF_CODEC_DISPATCHER_H

