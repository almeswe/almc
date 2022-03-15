#if defined(_WIN32) || defined(WIN32)
	#include "front-end/front-end.h"
	#include "back-end/back-end.h"
#else
	#error "This compiler is Windows-only."
#endif