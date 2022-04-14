#ifndef _ALMC_PRINTERR_H
#define _ALMC_PRINTERR_H

#include <stdio.h>

#define printerr(format, ...)					    \
	if (fprintf_s(stderr, format, __VA_ARGS__) < 0) \
		perror("printerr"), exit(1)

#endif // _ALMC_PRINTERR_H