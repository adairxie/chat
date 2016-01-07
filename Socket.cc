// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "Socket.h"

#include "InetAddress.h"
#include "SocketsOps.h"
#include "Logging.h"

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <strings.h>  // bzero

using namespace sockets;
Socket::~Socket()
{
  sockets::close(sockfd_);
}

void Socket::bindAddress(const InetAddress& addr)
{
  sockets::bindOrDie(sockfd_, addr.getSockAddr());
}

void Socket::listen()
{
  sockets::listenOrDie(sockfd_);
}

int Socket::accept(InetAddress* peeraddr)
{
  struct sockaddr_in6 addr;
  bzero(&addr, sizeof addr);
  int connfd = sockets::accept(sockfd_, &addr);
  if (connfd >= 0)
  {
    peeraddr->setSockAddrInet6(addr);
  }
  return connfd;
}

void Socket::setReuseAddr(bool on)
{
  int optval = on ? 1 : 0;
  ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR,
               &optval, sizeof optval);
  // FIXME CHECK
}

void Socket::setReusePort(bool on)
{
#ifdef SO_RUSEPORT
    int optival = on ? 1:0;
    int ret = ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT,
            &optval, static_cast<socklen_t>(sizeof optival));

    if (ret < 0 && on)
    {
        LOG_SYSERR << "SO_REUSEPORT failed.";
    }
#else
    if (on)
    {
        LOG_ERROR << "SO_REUSEPORT is not supported.";
    }
#endif
}

void Socket::shutdownWrite()
{
   sockets::shutdownWrite(sockfd_);
}

void Socket::setTcpNoDelay(bool on)
{
   int optval = on ? 1:0;
   ::setsockopt(sockfd_,IPPROTO_TCP,TCP_NODELAY,
	        &optval,sizeof optval);

   //FIXME CHECK
}

void Socket::setKeepAlive(bool on)
{
   int optval=on ? 1 : 0;
   ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE,
		&optval,static_cast<socklen_t>(sizeof optval));
}
