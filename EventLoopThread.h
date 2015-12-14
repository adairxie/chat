#ifndef _EVENTLOOPTHREAD_H
#define _EVENTLOOPTHREAD_H

#include "Thread.h"
#include "Mutex.h"
#include "Condition.h"

#include <boost/noncopyable.hpp>

class EventLoop;

class EventLoopThread : boost::noncopyable
{
  public:
     EventLoopThread();
     ~EventLoopThread();
     EventLoop* startLoop();
  private:
     void threadFunc();

     EventLoop* loop_;
     bool exiting_;
     Thread thread_;
     MutexLock mutex_;
     Condition cond_;
};



#endif
