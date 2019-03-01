#include "util.h"
#include "types.h"

#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>

typedef struct
{   std::string filename;
    std::string addr;
    pid_t       pid;
} clientparam;

#define CLIENTMAX 	16

static clientparam	clients[CLIENTMAX];


void clientutilinit()
{
	for (unsigned i=0; i < CLIENTMAX; ++i)
	{
		clients[i].pid = 0;
		clients[i].filename = "";
		clients[i].addr = "";
	}
}


bool clientutilnew(pid_t pid, std::string filename, std::string addr)
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
	

bool clientutildelete(pid_t pid)
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


int clientutilcount()
{
	unsigned j = 0;
	for (unsigned i=0; i < CLIENTMAX; ++i)
		if (clients[i].pid != 0 )
			++j;
	return j;
}


pid_t clientutilfind(std::string filename, std::string addr)
{
	for (unsigned i=0; i < CLIENTMAX; ++i)
		if ((clients[i].addr == addr) && (clients[i].filename == filename ))
		{
			Util::vlog("clientutil: found client pid: %d, addr: %s, filename: %s", clients[i].pid, clients[i].addr.c_str(), clients[i].filename.c_str());
			return clients[i].pid;
		}
	return 0;
}



