#ifndef _TIMERQUEUE_H
#define _TIMERQUEUE_H


#include <set>
#include <vector>

#include <boost/noncopyable.hpp>

#include "Timestamp.h"
#include "Mutex.h"
#include "Callbacks.h"
#include "Channel.h"
#include "Timer.h"
#include "TimerId.h"
class EventLoop;
class Timer;
class TimerId;

///
// A best efforts timer queue.
// No guarantee that the callbacks will be on time.
///

class TimerQueue:boost::noncopyable
{
   public:
     TimerQueue(EventLoop* loop);
     ~TimerQueue();

     TimerId addTimer(const TimerCallback& cb,
		     Timestamp when,
		     double interval);
     void cancel(TimerId timerId);
  private:
     
     //FIXME: use unique_ptr<Timer> instead of raw pointers.
     typedef std::pair<Timestamp,Timer*> Entry;
     typedef std::set<Entry> TimerList;
     typedef std::pair<Timer*, int64_t> ActiveTimer;
     typedef std::set<ActiveTimer> ActiveTimerSet;

     void addTimerInLoop(Timer* timer);
     void cancelInLoop(TimerId timerId);

     //called when timerfd alarm
     void handleRead();
     //move out all expired timers
     std::vector<Entry> getExpired(Timestamp now);
     void reset(const std::vector<Entry>& expired, Timestamp now);

     bool insert(Timer* timer);

     EventLoop* loop_;
     const int timerfd_;
     Channel timerfdChannel_;
     //Timer list sorted by expiration
     TimerList timers_;

     //for cancel()
     bool callingExpiredTimers_; /* atomic */
     ActiveTimerSet  activeTimers_;
     ActiveTimerSet  cancelingTimers_;
};



#endif

