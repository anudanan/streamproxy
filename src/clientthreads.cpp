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
#include <signal.h>
#include <linux/sockios.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <poll.h>
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

void *ClientThread::clientmain(void *)
{		
		pthread_t	tid;
		ThreadData *tdp;
		StreamingParameters::const_iterator spit;
		tid = pthread_self();

		for (;;)
		{
			usleep(100000);
//			Util::vlog("ClientThreads: thread id %d", tid);
			tdp = threadutil.findtid(tid);
//			Util::vlog("clientthread: tid: %d, addr: %s, filename: %s, fd: %d", tdp->tid, tdp->addr.c_str(), tdp->name.c_str(), tdp->fd);

			switch (tdp->tstate)		
			{
				case (st_filetrans):
			 	{	Util::vlog("clientthread: streaming parameters:");
					for(spit = tdp->streaming_parameters.begin(); spit != tdp->streaming_parameters.end(); spit++)
						Util::vlog("    %s = %s", spit->first.c_str(), spit->second.c_str());

					switch(tdp->stb_traits->transcoding_type)
					{
						case(stb_transcoding_broadcom):
						{
							Util::vlog("Clientthreads: transcoding service broadcom");
							(void)FileTranscodingBroadcom(tdp);
							break;
						}

//	       	        	case(stb_transcoding_enigma):
//         	      		{
//         		           	string service(string("1:0:1:0:0:0:0:0:0:0:") + Url(urlparams.at("file")).encode());
//
//             		       	Util::vlog("Clientthread: transcoding service enigma");
//                 		   	(void)TranscodingEnigma(service, fd, webauth, stb_traits, streaming_parameters);
//                  		break;
//						}
//
						default:
						{
							throw(http_trap(string("not a supported stb for transcoding"), 400, "Bad request"));
							Util::vlog("Clientthreads: not a supported stb for transcoding");
						}
					}

					threadutil.erasejob(tid);
					Util::vlog("ClientSocket: file transcoding ends");
					break;
				}

				case (st_livetrans):
				{
					Service service(tdp->name);

					Util::vlog("ClientSocket: live transcoding request");

					switch(tdp->stb_traits->transcoding_type)
					{	
                		case(stb_transcoding_broadcom):
                		{
                    		Util::vlog("ClientThreads: transcoding service broadcom");
                    		(void)LiveTranscodingBroadcom(service, tdp->fd, *(tdp->stb_traits), tdp->streaming_parameters, *(tdp->config_map));
						}
						break;

//	 					case(stb_transcoding_enigma):
//						{
//							Util::vlog("ClientThreads: transcoding service enigma");
//							(void)TranscodingEnigma(service.service_string(), fd, webauth, *stb_traits, streaming_parameters);
//							break;
//						}

 						default:
						{
//							throw(http_trap(string("not a supported stb for transcoding"), 400, "Bad request"));
							Util::vlog("Clientthreads: not a supported stb for transcoding");
		                }
   		         	}
					threadutil.erasejob(tid);
					Util::vlog("ClientSocket: live transcoding ends");
					break;
				}

				default:
				{
					break;
				}
			}
		}
}
