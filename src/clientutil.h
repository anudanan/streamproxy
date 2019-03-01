#ifndef _clientutil_h_
#define _clientutil_h_

#include "types.h"

#include <sys/types.h>
#include <string>

bool	clientutilnew(pid_t pid, std::string filename, std::string addr);
bool	clientutildelete(pid_t pid);
pid_t	clientutilfind(std::string filename, std::string addr);
int		clientutilcount();
void	clientutilinit();

#endif
