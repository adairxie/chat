#ifndef _CHANNEL_H
#define _CHANNEL_H	

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include "Timestamp.h"
class EventLoop;


class Channel: boost::noncopyable
{
   public:
     typedef boost::function<void()> EventCallback;
     typedef boost::function<void(Timestamp)> ReadEventCallback;
     Channel(EventLoop* loop,int fd);
     ~Channel();
     void handleEvent(Timestamp receiveTime);
     void setReadCallback(const ReadEventCallback& cb)
     { readCallback_=cb;}
     void setWriteCallback(const EventCallback& cb)
     { writeCallback_ = cb;}
     void setErrorCallback(const EventCallback& cb)
     { errorCallback_ =cb; }
     void setCloseCallback(const EventCallback& cb)
     { closeCallback_=cb;} 

     int fd() const { return fd_;}
     int events() const { return events_;}
     void set_revents(int revt) { revents_=revt;}
     bool isNoneEvent() const { return events_ == kNoneEvent; }

     void enableReading() { events_ |= kReadEvent; update(); }
     void enableWriting() { events_ |= kWriteEvent; update(); }
     void disableWriting() { events_ &= ~kWriteEvent; update();}
     void disableAll() { events_ = kNoneEvent; update();}
     bool isWriting() const { return events_ & kWriteEvent; }     
     //for Poller
     int index() { return index_;}
     void set_index(int idx) { index_ = idx ;}

     EventLoop* ownerLoop() {return loop_;}
     void remove();
  private:
     void update();

     static const int kNoneEvent;
     static const int kReadEvent;
     static const int kWriteEvent;

     EventLoop* loop_;
     const int fd_;
     int   events_;
     int   revents_;
     int  index_; //used by Poller.
     
     bool eventHandling_;
     bool addedToLoop_;

     ReadEventCallback readCallback_;
     EventCallback writeCallback_;
     EventCallback errorCallback_;
     EventCallback closeCallback_;
};

#endif 
