#ifndef _clientutil_h_
#define _clientutil_h_

#include "types.h"

#include <sys/types.h>
#include <string>

class	ClientUtil
{
	public:
		static	bool	create(pid_t pid, std::string filename, std::string addr);
		static	bool	erase(pid_t pid);
		static	pid_t	find(std::string filename, std::string addr);
		static	int		count();
		static	void	init();
};

#endif
