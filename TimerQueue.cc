#include "TimerQueue.h"

#include "Logging.h"
#include "EventLoop.h"
#include "Timer.h"
#include "TimerId.h"

#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <sys/timerfd.h>
//for debug howMushTimeFromNow
#include <iostream>

int createTimerfd()
{
   int timerfd=::timerfd_create(CLOCK_MONOTONIC,
		   TFD_NONBLOCK | TFD_CLOEXEC);

   if(timerfd < 0 )
   {
    LOG_SYSFATAL <<"Failed in timerfd_create";
   }
   return timerfd;
}

struct timespec howMuchTimeFromNow(Timestamp when)
{
  int64_t microseconds=when.microSecondsSinceEpoch()
	  - Timestamp::now().microSecondsSinceEpoch();
  if(microseconds < 100 )
  {
     microseconds=100;
  }
  struct timespec ts;
  ts.tv_sec=static_cast<time_t>(
		  microseconds / Timestamp::kMicroSecondsPerSecond);
  ts.tv_nsec=static_cast<long>(
		  (microseconds /Timestamp::kMicroSecondsPerSecond) * 1000);
  return ts;
}


void readTimerfd(int timerfd,Timestamp now)
{
   uint64_t howmany;
   ssize_t n=::read(timerfd,&howmany,sizeof howmany);
   LOG_TRACE << "TimerQueue::handleRead() " << howmany<<" at"<<now.toString();
   if( n!= sizeof howmany)
   {
      LOG_ERROR << "TimerQueue::handleRead() reads"<<n<< " bytes instead of 8";
   }
}

void resetTimerfd(int timerfd, Timestamp expiration)
{
   //wake up loop by timerfd_settime()
   struct itimerspec newValue;
   struct itimerspec oldValue;
   bzero(&newValue, sizeof newValue);
   bzero(&oldValue, sizeof oldValue);
   newValue.it_value=howMuchTimeFromNow(expiration);
   int ret=::timerfd_settime(timerfd,0,&newValue,&oldValue);
   if(ret)
   {
   LOG_SYSERR << "timerfd_settime()";
   }
}


TimerQueue::TimerQueue(EventLoop* loop)
	:loop_(loop),
	timerfd_(createTimerfd()),
	timerfdChannel_(loop,timerfd_),
	timers_(),
	callingExpiredTimers_(false)
{
  timerfdChannel_.setReadCallback(
		  boost::bind(&TimerQueue::handleRead,this));
  //we are always reading the timerfd,we disarm it with timefd_setttime.
  timerfdChannel_.enableReading();
}

TimerQueue::~TimerQueue()
{
   ::close(timerfd_);
   //do not remove channel,since we're in EventLoop::dtor();
   for(TimerList::iterator it=timers_.begin();
	  it != timers_.end(); ++it)
   {
	 delete it->second;
   }
}

TimerId TimerQueue::addTimer(const TimerCallback& cb,
		Timestamp when,
		double interval)
{
  Timer* timer=new Timer(cb,when,interval);
  loop_->runInLoop(
	 boost::bind(&TimerQueue::addTimerInLoop,this,timer));
  return TimerId(timer);
}

void TimerQueue::cancel(TimerId timerId)
{
  loop_->runInLoop(
       boost::bind(&TimerQueue::cancel,this,timerId));
}

void TimerQueue::cancelInLoop(TimerId timerId)
{
  loop_->assertInLoopThread();
  assert(timers_.size() == activeTimers_.size());
  ActiveTimer timer(timerId.timer_, timerId.sequence_);
  ActiveTimerSet::iterator it= activeTimers_.find(timer);
  if( it != activeTimers_.end())
  {
     size_t n = timers_.erase(Entry(it->first->expiration(),it->first));
     assert( n == 1); (void) n;
     delete it->first; 
     activeTimers_.erase(it);
  }
  else if (callingExpiredTimers_)
  {
     cancelingTimers_.insert(timer);
  }
  assert(timers_.size() == activeTimers_.size());
}
void TimerQueue::addTimerInLoop(Timer* timer)
{
  loop_->assertInLoopThread();
  bool earliestChanged = insert(timer);

  if(earliestChanged)
  { 
    resetTimerfd(timerfd_,timer->expiration());
  }
}


void TimerQueue::handleRead()
{
   loop_->assertInLoopThread();
   Timestamp now(Timestamp::now());
   readTimerfd(timerfd_,now);

   std::vector<Entry> expired=getExpired(now);
   
   callingExpiredTimers_ =true;
   cancelingTimers_.clear();
   //safe to callback outside critical section
   for(std::vector<Entry>::iterator it=expired.begin();
		   it!=expired.end(); ++it)
   {
      	   
      it->second->run();
   }
   callingExpiredTimers_= false;
   reset(expired,now);
}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now)
{
  assert(timers_.size() == activeTimers_.size());
  std::vector<Entry> expired;
  Entry sentry=make_pair(now,reinterpret_cast<Timer*>(UINTPTR_MAX));
  TimerList::iterator it=timers_.lower_bound(sentry);
  assert(it == timers_.end() || now < it->first);
  std::copy(timers_.begin(),timers_.end(),back_inserter(expired));
  timers_.erase(timers_.begin(), it);

  BOOST_FOREACH(Entry entry,expired)
  {
     ActiveTimer timer(entry.second, entry.second->sequence());
     size_t n = activeTimers_.erase(timer);
     assert(n == 1 ); (void)n;
  }

  assert(timers_.size() == activeTimers_.size());
  return expired;

}

void TimerQueue::reset(const std::vector<Entry>& expired, Timestamp now)
{
  Timestamp nextExpire;

  for(std::vector<Entry>::const_iterator it=expired.begin();
	 it!=expired.end(); ++it)
  {
     ActiveTimer timer(it->second,it->second->sequence());
     if(it->second->repeat()
             && cancelingTimers_.find(timer) == cancelingTimers_.end())
     {
	 it->second->restart(now);
	 insert(it->second);
     }
     else
     {
	//move to a free list
	delete it->second;
     }
  }

  if(!timers_.empty())
  {
    nextExpire=timers_.begin()->second->expiration();
  }

  if( nextExpire.valid())
  {
     resetTimerfd(timerfd_,nextExpire);
  }

}

bool TimerQueue::insert(Timer* timer)
{
  loop_->assertInLoopThread();
  assert( timers_.size() == activeTimers_.size());
  bool earliestChanged=false;
  Timestamp when=timer->expiration();
  TimerList::iterator it=timers_.begin();
  if(it == timers_.end() || when < it->first )
  {
    earliestChanged=true;
  }
  //set 的返回值型別是以pair組織起來的兩個值
  //pair 结构中的second成员表示安插是否成功
  //pair 结构中的first成员返回新元素的位置
  
  {
    std::pair<TimerList::iterator,bool> result
	    =timers_.insert(Entry(when,timer));
    assert(result.second); (void)result;
  }
  { 
     std::pair<TimerList::iterator,bool> result=
     timers_.insert(std::make_pair(when,timer));
     assert(result.second);
  }
  assert(timers_.size() == activeTimers_.size());
  return earliestChanged; 
}
