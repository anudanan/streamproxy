#ifndef _filetranscoding_broadcom_h_
#define _filetranscoding_broadcom_h_

#include "config.h"
#include "trap.h"
#include "types.h"
#include "configmap.h"
#include "stbtraits.h"
#include "threadutil.h"

#include <string>
#include <sys/types.h>

class FileTranscodingBroadcom
{
	private:

		enum
		{
			broadcom_magic_buffer_size = 256 * 188,
		};

		typedef enum
		{
			state_initial,
			state_starting,
			state_running
		} encoder_state_t;

		FileTranscodingBroadcom();
		FileTranscodingBroadcom(FileTranscodingBroadcom &);

		char *encoder_buffer;

	public:

		FileTranscodingBroadcom(ThreadData *tdp);
		~FileTranscodingBroadcom();
};

#endif
