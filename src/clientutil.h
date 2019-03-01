#ifndef _clientutil_h_
#define _clientutil_h_

#include "types.h"

#include <sys/types.h>
#include <string>

#define CLIENTMAX   16

class	ClientUtil
{	private:
		struct
		{	std::string	filename;
			std::string	addr;
			pid_t		pid;
		} clients[CLIENTMAX];

	public:
		ClientUtil();

		bool	create(pid_t pid, std::string filename, std::string addr);
		bool	erase(pid_t pid);
		pid_t	find(std::string filename, std::string addr);
		int		count();
};

extern ClientUtil clientutil;
#endif
