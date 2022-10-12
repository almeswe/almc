#ifndef _ALMC_H
#define _ALMC_H
	#define _ALMC_x86_64
	#ifdef __linux__
		#include "front-end/front-end.h"
		#include "back-end/back-end.h"
	#else
		#error "This platform is not supported."
	#endif
#endif