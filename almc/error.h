#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "context.h"

void report_error(const char* message, SrcContext* context);
void report_warning(const char* message, SrcContext* context);