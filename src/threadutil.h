#ifndef _threadutil_h_
#define _threadutil_h_

#include "types.h"
#include "configmap.h"
#include "types.h"
#include "stbtraits.h"

#include <sys/types.h>
#include <pthread.h>
#include <string>

#define CLIENTTHREADS  32

typedef enum
{
	st_idle,
	st_idletrans,
	st_filetrans,
	st_livetrans,
	st_filestream,
	st_livestream
} ThreadState;

typedef	struct
{
	pthread_t			tid;
	int					encodernum;
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
		ThreadData		*findfiletransseek(std::string filename, std::string addr, int fd, StreamingParameters streaming_parameters);
		ThreadData		*findfilestreamseek(std::string filename, std::string addr, int fd, StreamingParameters streaming_parameters);
		bool			jobsidle();
		bool			createfiletransjob(std::string filename, std::string addr, int fd, std::string webauth,
								const stb_traits_t *stb_traits, StreamingParameters streaming_parameters,
								const ConfigMap *config_map);
		bool			createfilestreamjob(std::string filename, std::string addr, int fd, std::string webauth,
								const stb_traits_t *stb_traits, StreamingParameters streaming_parameters,
								const ConfigMap *config_map);
		bool			createlivetransjob(std::string service, std::string addr, int fd, std::string webauth,
								const stb_traits_t *stb_traits, StreamingParameters streaming_parameters,
								const ConfigMap *config_map);
		bool			createlivestreamjob(std::string service, std::string addr, int fd, std::string webauth,
								const stb_traits_t *stb_traits, StreamingParameters streaming_parameters,
								const ConfigMap *config_map);
		void			erasejob(ThreadData* tdp);

};

extern ThreadUtil threadutil;
#endif
