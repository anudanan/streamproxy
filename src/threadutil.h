#ifndef _threadutil_h_
#define _threadutil_h_

#include "types.h"
#include "encoder-broadcom.h"
#include "configmap.h"
#include "config.h"
#include "trap.h"
#include "service.h"
#include "util.h"
#include "types.h"
#include "stbtraits.h"
#include "threadutil.h"

#include <sys/types.h>
#include <pthread.h>
#include <string>

#define CLIENTTHREADS  4 

typedef enum 
{ 
	st_none, 
	st_idle, 
	st_filetrans, 
	st_livetrans 
} ThreadState;

typedef	struct 
{	
	pthread_t			tid;
	ThreadState			tstate;
	int					seekposition;
	std::string			name;
	std::string			addr;
	int					fd;
	const stb_traits_t 		*stb_traits;
	StreamingParameters	streaming_parameters;
   	const ConfigMap 			*config_map;
} ThreadData;

class	ThreadUtil
{	private:
		ThreadData		clientthread[CLIENTTHREADS];

	public:
		ThreadUtil();

		bool			create(pthread_t tid);
		ThreadData		*findclientseek(std::string filename, std::string addr, int fd, StreamingParameters streaming_parameters);
		ThreadData		*findtid(pthread_t tid);
		bool			jobsidle();
		bool			createfilejob(std::string filename, std::string addr, int fd,
								const stb_traits_t *stb_traits, StreamingParameters streaming_parameters,
            					const ConfigMap *config_map);
		bool			createlivejob(std::string service, std::string addr, int fd,
								const stb_traits_t *stb_traits, StreamingParameters streaming_parameters,
            					const ConfigMap *config_map);
		bool			erasejob(pthread_t tid);

};

extern ThreadUtil threadutil;
#endif
