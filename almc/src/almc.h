#ifdef __unix__

#elif defined(_WIN32) || defined(WIN32)
	#define	 WINDOWS_OS
	//todo: probably target paltform should be selected in parse_options.
	// supported target platforms
	#define  __tgt_x86__
	//
	#include "front-end/front-end.h"

	#ifndef _M_IX86
		report_error("Cannot compile almc for this platform.", NULL);
	#endif
	#include "back-end/back-end.h"
#else
	report_error("This compiler is Windows-only.");
#endif