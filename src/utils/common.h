#ifndef _ALMC_COMMON_H
#define _ALMC_COMMON_H

#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "xmemory/xmemory.h"

#define _FRMT_BUFFER_SIZE       4096
#define _FRMT_BUFFER_MAX_LOAD   0.80

char _frmt_buffer[_FRMT_BUFFER_SIZE];

char* frmt(const char* format, ...);

size_t _get_frmt_buffer_ptr();

bool isidnt(char c);
bool isidnt_ext(char c);
bool isdigit_bin(char c);
bool isdigit_oct(char c);
bool isdigit_hex(char c);
bool isdigit_ext(char c);
bool isdigit_fext(char c);
bool isstrc(char c);
bool issquote(char c);
bool isdquote(char c);
bool issharp(char c);
bool isescape(char c);

#endif // _ALMC_COMMON_H