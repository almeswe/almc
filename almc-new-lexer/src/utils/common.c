#include "common.h"

char* frmt(const char* format, ...)
{
	int len;
	char* buffer;
	va_list args;

	va_start(args, format);
	len = _vscprintf(format, args) + 1;
	buffer = newc_s(char, buffer, len);
	if (NULL != buffer)
		vsprintf_s(buffer, len, format, args);
	va_end(args);
	return buffer ? buffer : new(char);
}