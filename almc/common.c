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

char* strcatc(char* source, char ch)
{
	char* new_str = malloc(sizeof(char) * (strlen(source) + 2));
	strcpy(new_str, source);
	size_t new_size = strlen(new_str);
	new_str[new_size] = ch;
	new_str[new_size + 1] = '\0';
	return new_str;
}

char* strcatcst(const char* str1, const char* str2)
{
	size_t new_len = strlen(str1) + strlen(str2);
	char* new_str = malloc(sizeof(char) * (new_len + 1));
	strcpy(new_str, str1);
	strcpy(new_str + strlen(str1), str2);
	return new_str;
}