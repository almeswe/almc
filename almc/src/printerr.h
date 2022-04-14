#ifndef ALMC_PRINTERR_H
#define ALMC_PRINTERR_H

#include <stdio.h>

#define printerr(format, ...)					    \
	if (fprintf_s(stderr, format, __VA_ARGS__) < 0) \
		perror("printerr"), exit(1)

#endif