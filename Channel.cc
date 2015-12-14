#include "Channel.h"
#include "EventLoop.h"
#include "Logging.h"

#include <sstream>

#include <poll.h>

const int Channel::kNoneEvent =0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop* loop,int fdArg)
	:loop_(loop),
	fd_(fdArg),
	events_(0),
	revents_(0),
	index_(-1),
	eventHandling_(false)
{
}

Channel::~Channel()
{
    assert(!eventHandling_);
}

void Channel::update()
{
    loop_->updateChannel(this);
}

void Channel::handleEvent(Timestamp receiveTime)
{
    eventHandling_ = true;
    if(revents_ & POLLNVAL){
    LOG_WARN << "Channel::handle_event() POLLINVAL";
    }
    
    if((revents_ & POLLHUP) && !(revents_ & POLLIN)){
	    LOG_WARN <<"Channel::handle_event() POLLHUP";
	    if(closeCallback_) closeCallback_();
    }

    if(revents_ & (POLLERR | POLLNVAL)){
       if( errorCallback_) errorCallback_();
    }
    if(revents_ & (POLLIN | POLLPRI | POLLRDHUP)) {
	if( readCallback_) readCallback_(receiveTime);
    }
    if(revents_ & POLLOUT){
        if( writeCallback_) writeCallback_();
    }
   eventHandling_ = false;
}
