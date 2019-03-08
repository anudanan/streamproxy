#include "config.h"
#include "trap.h"

#include "types.h"
#include "util.h"
#include "queue.h"
#include "transcoding-enigma.h"
#include "threadutil.h"

#include <string>
using std::string;

#include <ctype.h>
#include <unistd.h>
#include <poll.h>
#include <time.h>

#include <stdio.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <linux/sockios.h>
#include <stdlib.h>

static const struct addrinfo gai_localhost_hints =
{
	.ai_flags		= AI_NUMERICHOST,
	.ai_family		= AF_INET,
	.ai_socktype	= SOCK_STREAM,
	.ai_protocol	= 0,
	.ai_addrlen		= 0,
	.ai_addr		= 0,
	.ai_canonname	= 0,
	.ai_next		= 0,
};

TranscodingEnigma::TranscodingEnigma(const std::string &service, ThreadData* tdp)
//TranscodingEnigma::TranscodingEnigma(const string &service, int socketfd,
//		string webauth, const stb_traits_t &stb_traits,
//		const StreamingParameters &streaming_parameters)
{
	size_t				feature_index;
	const stb_feature_t	*feature = 0;
	string				value, bitrate, width, height;
	int					int_value;
	struct	addrinfo	*gai_localhost_8001;
	struct	linger		so_linger = { 0, 0 };
	int					rv;
	int					enigmafd;
	string				request;
	struct pollfd		pfd[2];
	Queue				socket_queue(1024 * 1024);
	string				httpok = "HTTP/1.0 200 OK\r\n"
							"Connection: Close\r\n"
							"Content-Type: video/mpeg\r\n"
							"\r\n";
	int					socketfd;

	Util::vlog("TranscodingEnigma: %s", service.c_str());

	for(StreamingParameters::const_iterator it(tdp->streaming_parameters.begin()); it != tdp->streaming_parameters.end(); it++)
	{
		for(feature_index = 0; feature_index < tdp->stb_traits->num_features; feature_index++)
		{
			feature = &(tdp->stb_traits->features[feature_index]);

			if(it->first == feature->id)
				break;
		}

		if(feature_index >= tdp->stb_traits->num_features)
		{
			Util::vlog("TranscodingEnigma: no stb traits/feature entry for streaming parameter \"%s\"", it->first.c_str());
			continue;
		}

		Util::vlog("TranscodingEnigma: found streaming parameter == stb_feature: \"%s\" [%s]", it->first.c_str(), it->second.c_str());

		if(!feature->settable)
		{
			Util::vlog("TranscodingEnigma: feature not settable, skip");
			continue;
		}

		switch(feature->type)
		{
			case(stb_traits_type_bool):
			{
				if((it->second == "off") || (it->second == "false") || (it->second == "0"))
					value = "off";
				else
					if((it->second == "on") || (it->second == "true") || (it->second == "1"))
						value = "on";
					else
					{
						Util::vlog("TranscodingEnigma: invalid bool value: \"\%s\"", it->second.c_str());
						continue;
					}

				break;
			}

			case(stb_traits_type_int):
			{
				int_value = strtol(it->second.c_str(), 0, 0);

				if(int_value < feature->value.int_type.min_value)
				{
					Util::vlog("TranscodingEnigma: integer value %s too small (%d)",
							it->second.c_str(), feature->value.int_type.min_value);
					continue;
				}

				if(int_value > feature->value.int_type.max_value)
				{
					Util::vlog("TranscodingEnigma: integer value %s too large (%d)",
							it->second.c_str(), feature->value.int_type.max_value);
					continue;
				}

				int_value *= feature->value.int_type.scaling_factor;
				value = Util::int_to_string(int_value);

				break;
			}

			case(stb_traits_type_string):
			{
				if(it->second.length() < feature->value.string_type.min_length)
				{
					Util::vlog("TranscodingEnigma: string value %s too short (%d)",
							it->second.c_str(), feature->value.string_type.min_length);
					continue;
				}

				if(it->second.length() > feature->value.string_type.max_length)
				{
					Util::vlog("TranscodingEnigma: string value %s too long (%d)",
							it->second.c_str(), feature->value.string_type.max_length);
					continue;
				}

				value = it->second;

				break;
			}

			case(stb_traits_type_string_enum):
			{
				const char * const *enum_value;

				for(enum_value = feature->value.string_enum_type.enum_values; *enum_value != 0; enum_value++)
					if(it->second == *enum_value)
						break;

				if(!*enum_value)
				{
					Util::vlog("TranscodingEnigma: invalid enum value: \"%s\"", it->second.c_str());
					continue;
				}

				value = *enum_value;

				break;
			}

			default:
			{
				Util::vlog("TranscodingEnigma: unknown feature type");
				return;
			}
		}

		if(it->first == "bitrate")
			bitrate = value;

		if(it->first == "size")
		{
			if(value == "480p")
			{
				width = "720";
				height = "480";
			}
			else if(value == "576p")
			{
				width = "720";
				height = "576";
			}
			else if(value == "720p")
			{
				width = "1280";
				height = "720";
			}
			else
			{
				width = "400";
				height = "300";
			}
		}
	}

	if((rv = getaddrinfo("0.0.0.0", "8001", &gai_localhost_hints, &gai_localhost_8001)))
	{
		Util::vlog("TranscodingEnigma: cannot get address for localhost:8001 %s", gai_strerror(rv));
		return;
	}

	if(!gai_localhost_8001)
	{
		Util::vlog("TranscodingEnigma: cannot get address for localhost:8001");
		return;
	}

	if((enigmafd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		freeaddrinfo(gai_localhost_8001);
		Util::vlog("TranscodingEnigma: cannot create socket");
		return;
	}

	if(setsockopt(enigmafd, SOL_SOCKET, SO_LINGER, &so_linger, sizeof(so_linger)))
	{
		freeaddrinfo(gai_localhost_8001);
		Util::vlog("TranscodingEnigma: cannot set linger");
		close(enigmafd);
		return;
	}

	if(connect(enigmafd, gai_localhost_8001->ai_addr, gai_localhost_8001->ai_addrlen))
	{
		freeaddrinfo(gai_localhost_8001);
		Util::vlog("TranscodingEnigma: cannot connect");
		close(enigmafd);
		return;
	}

	freeaddrinfo(gai_localhost_8001);

	request = string("GET /") +
				service + "?bitrate=" + bitrate + "?width=" + width + "?height=" + height + "?aspectratio=2?interlaced=0"
				" HTTP/1.0\r\n";

	if(tdp->webauth.length())
		request += "Authorization: Basic " + tdp->webauth + "\r\n";

	request += "\r\n";

	Util::vlog("TranscodingEnigma: send request to enigma: \"%s\"", request.c_str());

	if(write(enigmafd, request.c_str(), request.length()) != (ssize_t)request.length())
	{
		Util::vlog("TranscodingEnigma: cannot send request");
		close(enigmafd);
		return;
	}

	socket_queue.append(httpok.length(), httpok.c_str());

	socketfd = tdp->fd;

	for(;;)
	{
		if (tdp->fd != socketfd)           //  new seek request
		{
			close(tdp->fd);
			break;							// can´t not handle bei enigma
		}
		pfd[0].fd		= enigmafd;
		pfd[0].events	= POLLIN | POLLRDHUP;

		pfd[1].fd		= socketfd;
		pfd[1].events	= POLLRDHUP;

		if(socket_queue.length() > 0)
			pfd[1].events |= POLLOUT;

		if(poll(pfd, 2, -1) <= 0)
		{
			Util::vlog("TranscodingEnigma: streaming: poll error");
			break;
		}

		if(pfd[0].revents & (POLLERR | POLLHUP | POLLNVAL))
		{
			Util::vlog("TranscodingEnigma: enigma socket error");
			break;
		}

		if(pfd[1].revents & (POLLRDHUP | POLLHUP))
		{
			Util::vlog("TranscodingEnigma: client hung up");
			break;
		}

		if(pfd[0].revents & POLLIN)
		{
			if(!socket_queue.read(enigmafd))
			{
				Util::vlog("TranscodingEnigma: read enigma error");
				break;
			}
		}

		if(pfd[1].revents & POLLOUT)
		{
			if(!socket_queue.write(socketfd))
			{
				Util::vlog("TranscodingEnigma: write socket error");
				break;
			}
		}
	}

	close(enigmafd);

	Util::vlog("TranscodingEnigma: streaming ends\n");
}
