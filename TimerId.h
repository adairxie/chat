#ifndef _TIMERID_H
#define _TIMERID_H

#include "Timer.h"

class Timer;

class TimerId
{
  public: 
     TimerId(Timer* timer=NULL, int64_t seq=0)
	  :timer_(timer),
	  sequence_(seq)
     {
     }
  
   friend class TimerQueue;
  private:
     Timer* timer_;
     int64_t sequence_;
};

#endif
