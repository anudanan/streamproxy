#include "util.h"
#include "types.h"
#include "clientutil.h"

#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>

ClientUtil	clientutil;		// global class for client seek detection

ClientUtil::ClientUtil()	// Init
{
	for (unsigned i=0; i < CLIENTMAX; ++i)
	{
		clients[i].pid = 0;
		clients[i].filename = "";
		clients[i].addr = "";
	}
}

bool ClientUtil::create(pid_t pid, std::string filename, std::string addr)
{
	for (unsigned i=0; i < CLIENTMAX; ++i)
		if (clients[i].pid == 0)
		{	clients[i].pid = pid;
			clients[i].filename = filename;
			clients[i].addr = addr;
			Util::vlog("clientutil: new client pid: %d, addr: %s, filename: %s", clients[i].pid, clients[i].addr.c_str(), clients[i].filename.c_str());
			return true;
		}
	return false;
}

bool ClientUtil::erase(pid_t pid)
{
	for (unsigned i=0; i < CLIENTMAX; ++i)
		if (clients[i].pid == pid)
		{
			Util::vlog("clientutil: delete client pid: %d, addr: %s, filename: %s", clients[i].pid, clients[i].addr.c_str(), clients[i].filename.c_str());
			clients[i].pid = 0;
			clients[i].filename = "";
			clients[i].addr = "";
			return true;
		}
	return false;
}

int ClientUtil::count()
{
	unsigned j = 0;
	for (unsigned i=0; i < CLIENTMAX; ++i)
		if (clients[i].pid != 0 )
			++j;
	return j;
}

pid_t ClientUtil::find(std::string filename, std::string addr)
{
	for (unsigned i=0; i < CLIENTMAX; ++i)
		if ((clients[i].addr == addr) && (clients[i].filename == filename ))
		{
			Util::vlog("clientutil: found client pid: %d, addr: %s, filename: %s", clients[i].pid, clients[i].addr.c_str(), clients[i].filename.c_str());
			return clients[i].pid;
		}
	return 0;
}
