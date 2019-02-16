#include "config.h"
#include "trap.h"
#include "util.h"
#include "acceptsocket.h"

#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>

static const struct addrinfo gai_accept_hints =
{
	.ai_flags		= AI_PASSIVE,
	.ai_family		= AF_INET6,
	.ai_socktype	= SOCK_STREAM,
	.ai_protocol	= 0,
	.ai_addrlen		= 0,
	.ai_addr		= 0,
	.ai_canonname	= 0,
	.ai_next		= 0,
};

static const struct linger so_linger =
{
	.l_onoff	= 0,
	.l_linger	= 0,
};

AcceptSocket::AcceptSocket(string port)
{
	int rv;

	gai_accept_address = 0;

	if((fd = socket(AF_INET6, SOCK_STREAM | SOCK_CLOEXEC, 0)) < 0)
		Util::vlog("AcceptSocket: cannot create accept socket, port %s", port.c_str());
//		throw(trap("AcceptSocket: cannot create accept socket"));

	if(setsockopt(fd, SOL_SOCKET, SO_LINGER, &so_linger, sizeof(so_linger)))
		Util::vlog("AcceptSocket: cannot set linger on accept socket, port %s", port.c_str());
//		throw(trap("AcceptSocket: cannot set linger on accept socket"));

	rv = 1;
	if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &rv, sizeof(rv)))
		Util::vlog("AcceptSocket: cannot set reuseaddr on accept socket, port %s", port.c_str());
//		throw(trap("AcceptSocket: cannot set reuseaddr on accept socket"));

	if((rv = getaddrinfo(0, port.c_str(), &gai_accept_hints, &gai_accept_address)))
		Util::vlog("AcceptSocket: cannot get address info:i %s, port %s", gai_strerror(rv), port.c_str());
//		throw(trap(string("AcceptSocket: cannot get address info: ") + gai_strerror(rv)));

	if(!gai_accept_address)
		Util::vlog("AcceptSocket: cannot get address info:i %s, port %s", gai_strerror(rv), port.c_str());
//		throw(trap(string("AcceptSocket: cannot get address info: ") + gai_strerror(rv)));

	if(bind(fd, gai_accept_address->ai_addr, gai_accept_address->ai_addrlen))
		Util::vlog("AcceptSocket: cannot bind accept socket, port %s",port.c_str());
//		throw(trap("AcceptSocket: cannot bind accept socket"));

	if(listen(fd, 4))
		Util::vlog("AcceptSocket: cannot listen on accept socket, port %s",port.c_str());
//		throw(trap("AcceptSocket: cannot listen on accept socket"));

}

AcceptSocket::~AcceptSocket()
{
	if(gai_accept_address)
		freeaddrinfo(gai_accept_address);

	if(fd >= 0)
		close(fd);
}

int AcceptSocket::accept() const
{
	int new_fd;

	if((new_fd = ::accept(fd, 0, 0)) < 0)
		throw(trap("AcceptSocket: error in accept"));

	return(new_fd);
}

int AcceptSocket::get_fd() const
{
	return(fd);
}
