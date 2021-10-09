#ifndef ALMC_COMMON_H
#define ALMC_COMMON_H

#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>

#define new(type)			   (type*)malloc(sizeof(type))        
#define newc(type, count)	   (type*)malloc(sizeof(type) * (count))
#define cnew(type, count)	   (type*)calloc(count, sizeof(type))
#define rnew(type, count, var) (type*)realloc(var, sizeof(type) * (count))

#define new__err(var) (printf("Memory allocation failure [variable: %s, line: %d, file: %s]", #var, __LINE__, __FILE__), exit(1))
#define new__chk(var) var ? 0 : new__err(var)	
#define new_s(type, var)     new(type);     new__chk(var)
#define newc_s(type, var, c) newc(type, c); new__chk(var)
#define cnew_s(type, var, c) cnew(type, c); new__chk(var)
#define rnew_s(type, var, c) rnew(type, c, var); new__chk(var) 

char* frmt(const char* format, ...);

inline int isidnt(char c)
{
	return isalpha(c) || c == '_';
}
inline int isidnt_ext(char c)
{
	return isdigit(c) || isidnt(c);
}
inline int isdigit_bin(char c)
{
	return c == '0' || c == '1';
}
inline int isdigit_oct(char c)
{
	return isdigit(c) || (c >= '0' && c <= '7');
}
inline int isdigit_hex(char c)
{
	return isdigit(c) || ((c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'));
}
inline int isdigit_ext(char c)
{
	return isdigit(c) || c == '.' || c == 'e' || c == 'E';
}
inline int isdigit_fext(char c)
{
	return isdigit(c) || c == 'e' || c == 'E';
}


inline int isstrc(char c)
{
	return c != '\n' && c != '\"';
}
inline int issquote(char c)
{
	return c == '\'';
}
inline int isdquote(char c)
{
	return c == '\"';
}
inline int issharp(char c)
{
	return c == '#';
}
inline int isescape(char c)
{
	switch (c)
	{
	case '\a':
	case '\b':
	case '\f':
	case '\n':
	case '\r':
	case '\t':
	case '\v':
		return 1;
	}
	return 0;
}

#endif // COMMON_H