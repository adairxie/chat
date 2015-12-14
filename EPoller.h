#ifndef _EPOLLER_H
#define _EPOLLER_H

#include <map>
#include <vector>

#include "Timestamp.h"
#include "EventLoop.h"

struct epoll_event;

class Channel;

////
////IO multiplexing with epoll(4)
////
////This class doest't own the channel objects,

class EPoller: boost::noncopyable
{
 public:
   typedef std::vector<Channel*> ChannelList;

   EPoller(EventLoop* loop);
   ~EPoller();

   //Polls the I/O events.
   //Must be called in the loop thread.
   Timestamp poll(int timeoutMs,ChannelList* activeChannels);

   ///Changes the interested I/O events.
   ///Must be called in the loop thread.
   void updatedChannel(Channel* channel);
   ///Remove the channel,when it  destruct.
   void removeChannel(Channel* channel);

   void assertInLoopThread(){ ownerLoop_->assertInLoopThread(); }

  private:
   static const int KInitEventListSize =16;

   void fillActiveChannels(int numEvents,
		           Channnel* activeChannel);

  void update(int operations, Channel* channel);

  typedef std::vector<struct epoll_event> EventList;
  typedef std::map<int, Channel> ChannelMap;

  EventLoop* ownerLoop_;
  int epoll_fd;
  EventList events_;
  ChannelMap channels_;

};
#endif
