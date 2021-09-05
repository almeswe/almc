#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "context.h"

//todo: think about this scary macroses

#define lexer_raise_int_overflow_error() report_error("Integer size is too large!", NULL)

void report_error(const char* message, SrcContext* context);
void report_warning(const char* message, SrcContext* context);