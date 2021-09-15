#ifndef ALMC_ERROR_H
#define ALMC_ERROR_H

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "utils\context.h"

void report_error(const char* message, SrcContext* context);
void report_warning(const char* message, SrcContext* context);

#endif //ALMC_ERROR_H