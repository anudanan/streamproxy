#ifndef _filestreaming_h_
#define _filestreaming_h_

#include "config.h"
#include "trap.h"
#include "types.h"
#include "configmap.h"
#include "threadutil.h"

#include <string>
#include <sys/types.h>

class FileStreaming
{
	private:

		FileStreaming();
		FileStreaming(FileStreaming &);

	public:
		FileStreaming(ThreadData *tdp);
};

#endif
