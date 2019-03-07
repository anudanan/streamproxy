#include "config.h"
#include "trap.h"

#include "clientthreads.h"
#include "service.h"
#include "livestreaming.h"
#include "livetranscoding-broadcom.h"
#include "filestreaming.h"
#include "filetranscoding-broadcom.h"
#include "transcoding-enigma.h"
#include "util.h"
#include "url.h"
#include "types.h"
#include "webrequest.h"
#include "stbtraits.h"
#include "threadutil.h"

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <ctype.h>
#include <pwd.h>
#include <grp.h>
#include <shadow.h>
#include <pthread.h>

#include <string>
using std::string;
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/trim.hpp>

ClientThread::ClientThread()
{
}

void *ClientThread::clientfile(void *arg)
{
		ThreadData *tdp	= (ThreadData*) arg;

		Util::vlog("ClientSocket: file transcoding request");

		switch(tdp->stb_traits->transcoding_type)
		{
			case(stb_transcoding_broadcom):
			{
				Util::vlog("Clientthreads: transcoding service broadcom");
				(void)FileTranscodingBroadcom(tdp);
				break;
			}

			case(stb_transcoding_enigma):
			{
				string service(string("1:0:1:0:0:0:0:0:0:0:") + Url(tdp->name).encode());
				Util::vlog("Clientthread: transcoding service enigma");
//				(void)TranscodingEnigma(service, tdp->fd, tdp->webauth, *(tdp->stb_traits), tdp->streaming_parameters);
				(void)TranscodingEnigma(tdp);
				break;
			}

			default:
			{
//				throw(http_trap(string("not a supported stb for transcoding"), 400, "Bad request"));
				Util::vlog("Clientthreads: not a supported stb for transcoding");
			}
		}

		threadutil.erasejob(tdp);
		Util::vlog("ClientSocket: file transcoding ends");
		return NULL;
}

void *ClientThread::clientlive(void *arg)
{
		ThreadData *tdp	= (ThreadData*) arg;
		Service service(tdp->name);

		Util::vlog("ClientSocket: live transcoding request");

		switch(tdp->stb_traits->transcoding_type)
		{
			case(stb_transcoding_broadcom):
			{
				Util::vlog("ClientThreads: transcoding service broadcom");
				(void)LiveTranscodingBroadcom(service, tdp->fd, tdp->webauth, *(tdp->stb_traits), tdp->streaming_parameters, *(tdp->config_map));
				break;
			}

			case(stb_transcoding_enigma):
			{
				Util::vlog("ClientThreads: transcoding service enigma");
//				(void)TranscodingEnigma(service.service_string(), fd, webauth, *stb_traits, streaming_parameters);
				(void)TranscodingEnigma(tdp);
				break;
			}

			default:
			{
//				throw(http_trap(string("not a supported stb for transcoding"), 400, "Bad request"));
				Util::vlog("Clientthreads: not a supported stb for transcoding");
			}
		}
		threadutil.erasejob(tdp);
		Util::vlog("ClientSocket: live transcoding ends");
		return NULL;
}
