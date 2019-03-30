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
		clientthread[1].encodernum = -1;
		clientthread[i].tstate = st_idle;
		clientthread[i].name = "";
		clientthread[i].addr = "";
	}
}

bool ThreadUtil::createtransidle(int encodernum)
{
	for (unsigned i=0; i < CLIENTTHREADS; ++i)
		if (clientthread[i].tstate  == st_idle)
		{
			clientthread[i].tstate = st_idletrans,
			clientthread[i].encodernum = encodernum;
			Util::vlog("ThreadUtil: new trancode thread handel [%d], addr: %s, name: %s", i, clientthread[i].addr.c_str(), clientthread[i].name.c_str());
			return true;
		}
	return false;
}

bool ThreadUtil::createfiletransjob(std::string filename, std::string addr, int fd, std::string webauth,
                                const stb_traits_t *stb_traits, StreamingParameters streaming_parameters,
                                const ConfigMap *config_map)
{
	for (unsigned i=0; i < CLIENTTHREADS; ++i)
		if (clientthread[i].tstate == st_idletrans)
		{
			clientthread[i].name = filename;
			clientthread[i].addr = addr;
			clientthread[i].fd = fd;
			clientthread[i].webauth = webauth;
			clientthread[i].stb_traits = stb_traits;
			clientthread[i].streaming_parameters = streaming_parameters;
			clientthread[i].config_map = config_map;
			clientthread[i].tstate = st_filetrans;
			if(pthread_create(&(clientthread[i].tid), NULL, ClientThread::clientfiletrans, clientthread+i))
				throw(trap("cannot create streaming thread"));
			pthread_detach(clientthread[i].tid);

			Util::vlog("ThreadUtil: create filetrans job thread [%d], tid %ul, addr: %s, filename: %s d, fd: %d", i, clientthread[i].tid,
								clientthread[i].addr.c_str(), clientthread[i].name.c_str(), clientthread[i].fd);
			return true;
		}
	return false;
}

ThreadData *ThreadUtil::findfiletransseek(std::string filename, std::string addr, int fd, StreamingParameters streaming_parameters)
{
	for (unsigned i=0; i < CLIENTTHREADS; ++i)
		if ((clientthread[i].tstate == st_filetrans) && (clientthread[i].addr == addr) && (clientthread[i].name == filename ))
		{
			Util::vlog("ThreadUtil: found transseek thread [%d]:, addr: %s, filename: %s", i, clientthread[i].addr.c_str(), clientthread[i].name.c_str());
			clientthread[i].streaming_parameters = streaming_parameters;
			clientthread[i].fd = fd;
			return clientthread + i;
		}
	return NULL;
}

bool ThreadUtil::createfilestreamjob(std::string filename, std::string addr, int fd, std::string webauth,
                                const stb_traits_t *stb_traits, StreamingParameters streaming_parameters,
                                const ConfigMap *config_map)
{
	for (unsigned i=0; i < CLIENTTHREADS; ++i)
		if (clientthread[i].tstate == st_idle)
		{
			clientthread[i].name = filename;
			clientthread[i].addr = addr;
			clientthread[i].fd = fd;
			clientthread[i].webauth = webauth;
			clientthread[i].stb_traits = stb_traits;
			clientthread[i].streaming_parameters = streaming_parameters;
			clientthread[i].config_map = config_map;
			clientthread[i].tstate = st_filestream;
			if(pthread_create(&(clientthread[i].tid), NULL, ClientThread::clientfilestream, clientthread+i))
				throw(trap("cannot create streaming thread"));
			pthread_detach(clientthread[i].tid);

			Util::vlog("ThreadUtil: create filestream job thread [%d], tid %ul, addr: %s, filename: %s d, fd: %d", i, clientthread[i].tid,
								clientthread[i].addr.c_str(), clientthread[i].name.c_str(), clientthread[i].fd);
			return true;
		}
	return false;
}

ThreadData *ThreadUtil::findfilestreamseek(std::string filename, std::string addr, int fd, StreamingParameters streaming_parameters)
{
	for (unsigned i=0; i < CLIENTTHREADS; ++i)
		if ((clientthread[i].tstate == st_filestream) && (clientthread[i].addr == addr) && (clientthread[i].name == filename ))
		{
			Util::vlog("ThreadUtil: found fileseek thread [%d]:, addr: %s, filename: %s", i, clientthread[i].addr.c_str(), clientthread[i].name.c_str());
			clientthread[i].streaming_parameters = streaming_parameters;
			clientthread[i].fd = fd;
			return clientthread + i;
		}
	return NULL;
}

bool ThreadUtil::createlivetransjob(std::string service, std::string addr, int fd, std::string webauth,
                                const stb_traits_t *stb_traits, StreamingParameters streaming_parameters,
                                const ConfigMap *config_map)
{
	for (unsigned i=0; i < CLIENTTHREADS; ++i)
		if (clientthread[i].tstate == st_idletrans)
		{
			clientthread[i].name = service;
			clientthread[i].addr = addr;
			clientthread[i].fd = fd;
			clientthread[i].webauth = webauth;
			clientthread[i].stb_traits = stb_traits;
			clientthread[i].streaming_parameters = streaming_parameters;
			clientthread[i].config_map = config_map;
			clientthread[i].tstate = st_livetrans;
			if(pthread_create(&(clientthread[i].tid), NULL, ClientThread::clientlivetrans, clientthread+i))
				throw(trap("cannot create streaming thread"));
			pthread_detach(clientthread[i].tid);

			Util::vlog("ThreadUtil: create livetrans job thread [%d], tid %ul, addr: %s, service: %s d, fd: %d", i, clientthread[i].tid,
						clientthread[i].addr.c_str(), clientthread[i].name.c_str(), clientthread[i].fd);
			return true;
		}
	return false;
}

bool ThreadUtil::createlivestreamjob(std::string service, std::string addr, int fd, std::string webauth,
                                const stb_traits_t *stb_traits, StreamingParameters streaming_parameters,
                                const ConfigMap *config_map)
{
	for (unsigned i=0; i < CLIENTTHREADS; ++i)
		if (clientthread[i].tstate == st_idle)
		{
			clientthread[i].name = service;
			clientthread[i].addr = addr;
			clientthread[i].fd = fd;
			clientthread[i].webauth = webauth;
			clientthread[i].stb_traits = stb_traits;
			clientthread[i].streaming_parameters = streaming_parameters;
			clientthread[i].config_map = config_map;
			clientthread[i].tstate = st_livestream;
			if(pthread_create(&(clientthread[i].tid), NULL, ClientThread::clientlivestream, clientthread+i))
				throw(trap("cannot create streaming thread"));
			pthread_detach(clientthread[i].tid);

			Util::vlog("ThreadUtil: create livestream job thread [%d], tid %ul, addr: %s, service: %s d, fd: %d", i, clientthread[i].tid,
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
			if ((tdp->tstate == st_filetrans) || (tdp->tstate == st_livetrans))
				tdp->tstate = st_idletrans;
			else
				tdp->tstate = st_idle;
}

bool ThreadUtil::jobsidle()
{
	bool ret = true;
	for (unsigned i=0; i < CLIENTTHREADS; ++i)
		if ((clientthread[i].tstate != st_idle) && (clientthread[i].tstate != st_idletrans)) 
			ret = false;
	return ret;
}

