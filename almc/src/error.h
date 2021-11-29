#ifndef ALMC_ERROR_H
#define ALMC_ERROR_H

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "utils\context.h"

char** create_spell_lines(SrcContext* context);

void report_error(const char* message, SrcContext* context);
void report_error2(const char* message, SrcArea* area);
void report_warning(const char* message, SrcContext* context);
void report_warning2(const char* message, SrcArea* area);

#endif