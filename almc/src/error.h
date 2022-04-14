#ifndef _ALMC_ERROR_H
#define _ALMC_ERROR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "printerr.h"
#include "utils\context.h"
#include "utils\console\colors.h"

typedef char* (any_context_to_str)(void*);

void report_error(const char* message,
	const SrcContext* context);
void report_warning(const char* message,
	const SrcContext * context);
void report_error2(const char* message,
	const SrcArea* area);
void report_warning2(const char* message,
	const SrcArea* area);

void report_info_base(const char* header, const char* message, const char** spelling,
	any_context_to_str to_str_func, const void* any_context);

#endif // _ALMC_ERROR_H