#include "config.h"

#include "demuxer.h"
#include "types.h"
#include "util.h"

#include <string>
using std::string;

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdint.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/dvb/dmx.h>
#include <poll.h>

Demuxer::Demuxer(int id_in, const PidMap &pidmap)
{
	PidMap::const_iterator	demux_it;
	PidMap::const_iterator	it;
	PidMap::iterator		it2;

	struct		dmx_pes_filter_params dmx_filter;
	string		demuxer_device;
	uint16_t	pid;

	id = id_in;

	for(it = pidmap.begin(); it != pidmap.end(); it++)
	{
		for(it2 = pids.begin(); it2 != pids.end(); it2++)
			if(it2->second == it->second)
				break;

		if(it2 == pids.end())
			pids[it->first] = it->second;
		else
		{
			if(it->first == "video")
			{
				pids.erase(it2);
				pids[it->first] = it->second;
			}
		}
	}

	if((pids.find("pat") == pids.end()) || (pids.find("pmt") == pids.end()) ||
			(pids.find("video") == pids.end()) || (pids.find("audio") == pids.end()))
		throw(trap("Demuxer: missing primary pids"));

	demuxer_device = string("/dev/dvb/adapter0/demux") + Util::int_to_string(id);

	if((fd = open(demuxer_device.c_str(), O_RDWR | O_NONBLOCK)) < 0)
		throw(trap("Demuxer: cannot open demuxer device"));

	if(ioctl(fd, DMX_SET_BUFFER_SIZE, buffer_size))
		throw(trap("Demuxer: cannot set buffer size"));

	dmx_filter.pid		= pids["pat"];
	dmx_filter.input	= DMX_IN_FRONTEND;
	dmx_filter.output	= DMX_OUT_TSDEMUX_TAP;
	dmx_filter.pes_type	= DMX_PES_OTHER;
	dmx_filter.flags	= DMX_IMMEDIATE_START;

	if(ioctl(fd, DMX_SET_PES_FILTER, &dmx_filter))
		throw(trap("Demuxer: cannot set pes filter"));

	for(it = pids.begin(); it != pids.end(); it++)
	{
		if(it->first == "pat")
			continue;

		pid = it->second;
		Util::vlog("Demuxer: ioctl demuxer ADD PID: %s -> 0x%x", it->first.c_str(), pid);

		if(ioctl(fd, DMX_ADD_PID, &pid))
			throw(trap(string("Demuxer: cannot add pid for ") + it->first));
	}
}

Demuxer::~Demuxer()
{
	struct pollfd pfd;
	static char buffer[4096];
	ssize_t rv;

	Util::vlog("Demuxer: demuxer STOP, start draining");

	for(;;)
	{
		pfd.fd = fd;
		pfd.events = POLLIN;

		if((rv = poll(&pfd, 1, 10)) == 0)
			break;

		if(rv < 0)
		{
			Util::vlog("Demuxer: poll error");
			break;
		}

		if(pfd.revents & POLLIN)
		{
			rv = read(fd, buffer, sizeof(buffer));
			//Util::vlog("Demuxer: drained %d bytes", rv);

			if(rv <= 0)
				break;
		}
		else
			break;
	}

	Util::vlog("Demuxer: demuxer STOP, draining done");
}

int Demuxer::getfd() const
{
	return(fd);
}
