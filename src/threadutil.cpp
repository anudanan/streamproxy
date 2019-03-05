#include "util.h"
#include "types.h"
#include "threadutil.h"
#include "clientthreads.h"
#include "trap.h"

#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <pthread.h>

ThreadUtil	threadutil;			// global class for streaming thread handlung and client seek detection

ThreadUtil::ThreadUtil()		// Init
{
	for (unsigned i=0; i < CLIENTTHREADS; ++i)
	{
		clientthread[i].tid = 0;
		clientthread[i].tstate = st_none;
		clientthread[i].name = "";
		clientthread[i].addr = "";
	}
	clientthread[0].tstate = st_idle;
}

bool ThreadUtil::create(pthread_t tid)
{
	for (unsigned i=0; i < CLIENTTHREADS; ++i)
		if (clientthread[i].tid == 0)
		{
			clientthread[i].name = "";
			clientthread[i].addr = "";
			clientthread[i].tid = tid;
			clientthread[i].tstate = st_idle,
			Util::vlog("ThreadUtil: new thread [%d], tid: %ul, addr: %s, name: %s", i, clientthread[i].tid, clientthread[i].addr.c_str(), clientthread[i].name.c_str());
			return true;
		}
	return false;
}

bool ThreadUtil::createfilejob(std::string filename, std::string addr, int fd,
                                const stb_traits_t *stb_traits, StreamingParameters streaming_parameters,
                                const ConfigMap *config_map)
{
	for (unsigned i=0; i < CLIENTTHREADS; ++i)
		if (clientthread[i].tstate == st_idle)
		{
			clientthread[i].name = filename;
			clientthread[i].addr = addr;
			clientthread[i].fd = fd;
			clientthread[i].stb_traits = stb_traits;
			clientthread[i].streaming_parameters = streaming_parameters;
			clientthread[i].config_map = config_map;
			clientthread[i].tstate = st_filetrans;
			if(pthread_create(&(clientthread[i].tid), NULL, ClientThread::clientfile, clientthread+i))
				throw(trap("cannot create streaming thread"));

			Util::vlog("ThreadUtil: create file job thread [%d], tid %ul, addr: %s, filename: %s d, fd: %d", i, clientthread[i].tid,
								clientthread[i].addr.c_str(), clientthread[i].name.c_str(), clientthread[i].fd);
			return true;
		}
	return false;
}

bool ThreadUtil::createlivejob(std::string service, std::string addr, int fd,
                                const stb_traits_t *stb_traits, StreamingParameters streaming_parameters,
                                const ConfigMap *config_map)
{
	for (unsigned i=0; i < CLIENTTHREADS; ++i)
		if (clientthread[i].tstate == st_idle)
		{
			clientthread[i].name = service;
			clientthread[i].addr = addr;
			clientthread[i].fd = fd;
			clientthread[i].stb_traits = stb_traits;
			clientthread[i].streaming_parameters = streaming_parameters;
			clientthread[i].config_map = config_map;
			clientthread[i].tstate = st_livetrans;
			if(pthread_create(&(clientthread[i].tid), NULL, ClientThread::clientlive, clientthread+i))
				throw(trap("cannot create streaming thread"));

			Util::vlog("ThreadUtil: create live job thread [%d], tid %ul, addr: %s, service: %s d, fd: %d", i, clientthread[i].tid,
						clientthread[i].addr.c_str(), clientthread[i].name.c_str(), clientthread[i].fd);
			return true;
		}
	return false;
}

void ThreadUtil::erasejob(ThreadData *tdp)
{
		Util::vlog("ThreadUtil: erase job thread [%d], tid %ul, addr: %s, name: %s", tdp-clientthread, tdp->tid,
						tdp->addr.c_str(), tdp->name.c_str());
			tdp->name = "";
			tdp->addr = "";
			tdp->fd = 0;
			tdp->tstate = st_idle;
}

bool ThreadUtil::jobsidle()
{
	bool ret = true;
	for (unsigned i=0; i < CLIENTTHREADS; ++i)
		if ((clientthread[i].tstate != st_idle) && (clientthread[i].tstate != st_none))
			ret = false;
	return ret;
}

ThreadData *ThreadUtil::findclientseek(std::string filename, std::string addr, int fd, StreamingParameters streaming_parameters)
{
	for (unsigned i=0; i < CLIENTTHREADS; ++i)
		if ((clientthread[i].tstate == st_filetrans) && (clientthread[i].addr == addr) && (clientthread[i].name == filename ))
		{
			Util::vlog("ThreadUtil: found seek thread [%d]:, addr: %s, filename: %s", i, clientthread[i].addr.c_str(), clientthread[i].name.c_str());
			clientthread[i].streaming_parameters = streaming_parameters;
			clientthread[i].fd = fd;
			return clientthread + i;
		}
	return NULL;
}

ThreadData *ThreadUtil::findtid(pthread_t tid)
{
	for (unsigned i=0; i < CLIENTTHREADS; ++i)
		if (clientthread[i].tid == tid)
			return clientthread + i;
	return NULL;
}

