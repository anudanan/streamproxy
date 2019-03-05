#ifndef _clientthreads_h_
#define _clientthreads_h_

#include "config.h"

#include "types.h"
#include "configmap.h"
#include "stbtraits.h"

#include <stdint.h>
#include <string>
#include <vector>
#include <map>

class ClientThread
{
	private:

		typedef std::vector<std::string> stringvector;

	public:
		static void *clientfile(void*);
		static void *clientlive(void*);

		ClientThread();
//		~ClientThread();
};

#endif
