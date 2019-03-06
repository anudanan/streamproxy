#ifndef _threadutil_h_
#define _threadutil_h_

#include "types.h"
#include "configmap.h"
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
	std::string			name;
	std::string			addr;
	std::string			webauth;
	int					fd;
	const stb_traits_t	*stb_traits;
	StreamingParameters	streaming_parameters;
	const ConfigMap		*config_map;
} ThreadData;

class	ThreadUtil
{	private:
		ThreadData		clientthread[CLIENTTHREADS];

	public:
		ThreadUtil();

		bool			createtransidle();
		ThreadData		*findclientseek(std::string filename, std::string addr, int fd, StreamingParameters streaming_parameters);
		ThreadData		*findtid(pthread_t tid);
		bool			jobsidle();
		bool			createfilejob(std::string filename, std::string addr, int fd, std::string webauth,
								const stb_traits_t *stb_traits, StreamingParameters streaming_parameters,
								const ConfigMap *config_map);
		bool			createlivejob(std::string service, std::string addr, int fd, std::string webauth,
								const stb_traits_t *stb_traits, StreamingParameters streaming_parameters,
								const ConfigMap *config_map);
		void			erasejob(ThreadData* tdp);

};

extern ThreadUtil threadutil;
#endif
