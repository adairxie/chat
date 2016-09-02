#include <stdio.h>
#include <cstdlib>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>

#include "codec/codec.h"
#include "codec/dispatcher.h"
#include "../../TcpConnection.h"
#include "msg.pb.h"
#include "user.h"

#include <vector>
#include <boost/shared_ptr.hpp>

std::vector<TcpConnectionPtr>  connections;

using namespace im; //message namespace

/* every client sends this data to server */
static const char* request = "sdasdad";


int setnonblocking(int fd)
{
	int old_option = fcntl(fd, F_GETFL);
	int new_option = old_option | O_NONBLOCK;
	int ret = fcntl(fd, F_SETFL, new_option);
	return ret;
}

void addfd(int epoll_fd, int fd)
{
	struct epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLOUT | EPOLLET | EPOLLERR;
	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event);
	setnonblocking(fd);
}

/* write len bytes to server */
bool write_nbytes(int sockfd, const char* buffer, int len)
{
	int bytes_write=0;
	printf("write out %d bytes to socket %d\n", len, sockfd);
	while (1)
	{
		bytes_write = send(sockfd, buffer, len, 0);
		if (bytes_write == -1)
		{
			return false;
		}
		else if ( bytes_write == 0)
		{
			return false;
		}

		len -= bytes_write;
		buffer = buffer + bytes_write;
		if (len <= 0) 
		{
			return true;
		}

	}
}

/* recv data from server */
bool read_once(int sockfd, char* buffer, int len)
{
	int bytes_read = 0;
	memset(buffer, '\0', len);
	bytes_read = recv(sockfd, buffer, len, 0);
	if (bytes_read == -1)
	{
		return false;
	}
	else if (bytes_read == 0) 
	{
		return false;
	}
	printf("read in %d bytes from socket %d with content: %s\n", bytes_read, sockfd, buffer);

	return true;
}

/* start connect to server with num connect(), we can change the num to adjust the stress test */
void start_conn(int epoll_fd, int num, const char* ip, int port)
{
	struct sockaddr_in address;
	bzero(&address, sizeof(address));
	address.sin_family = AF_INET;
	inet_pton(AF_INET, ip, &address.sin_addr);
	address.sin_port = htons(port);

	for (int i = 0; i < num; ++i)
	{
		sleep(1);
		int sockfd = socket(AF_INET, SOCK_STREAM, 0);
		printf("create 1 sock\n");
		if (sockfd < 0)
		{
			continue;
		}

		if ( connect(sockfd, (struct sockaddr* )&address, sizeof(address) ) == 0)
		{
			Login lmsg;
			lmsg.set_uid(i);
			lmsg.set_passwd("123");
			printf( "build connection %d\n", i);
			connections.push_back(
			addfd(epoll_fd, sockfd);
		}
	}
}

void close_conn(int epoll_fd, int sockfd)
{
	epoll_ctl(epoll_fd, EPOLL_CTL_DEL, sockfd, 0);
	close(sockfd);
}


int main(int argc, char* argv[])
{
	assert( argc == 4);
	int epoll_fd = epoll_create(100);
	start_conn(epoll_fd, atoi(argv[3]), argv[1], atoi(argv[2]));
	struct epoll_event events[1000];
	char buffer[2048];
	while (1)
	{
		int fds = epoll_wait(epoll_fd, events, 10000, 2000);
		for (int i = 0; i < fds; i++)
		{
			int sockfd = events[i].data.fd;
			if (events[i].events & EPOLLIN) 
			{
				if (!read_once(sockfd, buffer, 2048))
				{
					close_conn(epoll_fd, sockfd);
				}
				struct epoll_event event;
				event.events = EPOLLOUT | EPOLLET | EPOLLERR;
				event.data.fd = sockfd;
				epoll_ctl(epoll_fd, EPOLL_CTL_MOD, sockfd, &event);
			}
			else if (events[i].events & EPOLLOUT) 
			{
				if ( !write_nbytes(sockfd, request, strlen(request)))
				{
					close_conn(epoll_fd, sockfd);
				}
				struct epoll_event event;
				event.events = EPOLLIN | EPOLLET | EPOLLERR;
				event.data.fd = sockfd;
				epoll_ctl(epoll_fd, EPOLL_CTL_MOD, sockfd, &event);
			}
			else if (events[i].events & EPOLLERR)
			{
				close_conn(epoll_fd, sockfd);
			}
		}
	}
}
