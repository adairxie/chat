#ifndef __THREAD_H
#define __THREAD_H
#include "Thread.h"
#include <boost/scoped_ptr.hpp>
#include <vector>
#include "Timestamp.h"
#include "TimerId.h"
#include "Callbacks.h"
#include "Mutex.h"

class Channel;
class Poller;
class TimerQueue;




class EventLoop:boost::noncopyable
{
  public:
    typedef boost::function<void()> Functor;

    EventLoop();
    ~EventLoop();

    void loop();


    void quit();
    ///
    /// Time when poll return,usually means data arrival.
    ///
    Timestamp pollReturnTime() const { return pollReturnTime_;}
    ///Runs callback immediatally in the thread.
    //It wakes up the loop,and run the cb.
    //If in the same loop thread,cb is run within the function.
    //Safe to call in other threads.
    void runInLoop(const Functor& cb);
    ///Queues callback in the loop thread.
    ///Runs after finish the polling.
    ///Safe in the other threads.
    void queueInLoop(const Functor& cb);
    ///Timers

    ///
    ///Runs callback at 'time'.
    ///
    TimerId runAt(const Timestamp& time,const TimerCallback& cb);
    ///

    ///
    ///Runs callback after @c delay seconds.
    ///
    TimerId runAfter(double delay,const TimerCallback& cb);
    ///
    ///Runs callback every @c interval seconds.
    ///
    TimerId runEvery(double interval,const TimerCallback& cb);

    void cancel(TimerId timerId);

    //inertal use only
    void wakeup();
    void updateChannel(Channel* channel);
    void removeChannel(Channel* channek);

    void assertInLoopThread()
    {
      if(!isInLoopThread())
      {
        abortNotInLoopThread();
      }
    }
    bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }
		pid_t getThreadId() const { return threadId_; }
    static EventLoop* getEventLoopOfCurrentThread();
  private:

    void abortNotInLoopThread();
    void handleRead();
    void doPendingFunctors();

    typedef std::vector<Channel*> ChannelList;



    bool looping_ ;  /* atomic */
    bool quit_;     /* atomic */
    bool callingPendingFunctors_; /* atomic */

    const pid_t threadId_;
    Timestamp pollReturnTime_;
    boost::scoped_ptr<Poller> poller_;
    boost::scoped_ptr<TimerQueue> timerQueue_;
    int wakeupFd_;
    //unlike in TimerQueue,which is an interval class
    //we don't expose Channel to client
    boost::scoped_ptr<Channel> wakeupChannel_;
    ChannelList activeChannels_;
    MutexLock mutex_;
    std::vector<Functor> pendingFunctors_;  //@GuardedBy mutex_

};
#endif
