#ifndef __POLLER_H
#define __POLLER_H



#include <map>
#include <vector>

#include  "Timestamp.h"
#include "EventLoop.h"

struct pollfd;

class Channel;

///
/// IO Multiplexing with poll(2).
///
//This class does't own the Channel objects.


class Poller: boost::noncopyable
{
   public:
      typedef std::vector<Channel*> ChannelList;
      Poller(EventLoop* loop);
      ~Poller();

      ///Polls the I/O events.
      ///Must be called in the loop thread.
      Timestamp poll(int timesoutMs, ChannelList* activeChannels);

      //Changes the interested I/O events.
      //Must be called in the loop thread.
      void updateChannel(Channel* channel);
      //Remove the channel,when it destructs.
      //Must be called in the loop thread.
      void removeChannel(Channel* channel);

      void assertInLoopThread() { ownerLoop_->assertInLoopThread();}

  private:
     void fillActiveChannels(int numEvents,ChannelList* activeChannels) const;
     typedef std::vector<struct pollfd> PollFdList;
     typedef std::map<int , Channel*> ChannelMap;

     EventLoop* ownerLoop_;
     PollFdList   pollfds_;
     ChannelMap channels_;

};

#endif
