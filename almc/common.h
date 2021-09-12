#ifndef ALMC_COMMON_H
#define ALMC_COMMON_H

#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>

#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[37m"      /* White */
#define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */

#define new(type)         (type*)malloc(sizeof(type));        
#define newc(type, count) (type*)malloc(sizeof(type) * count);

#define new__err(var) (printf("Memory allocation failure [variable: %s, line: %d, file: %s]", #var, __LINE__, __FILE__), exit(1))
#define new__chk(var) var ? 0 : new__err(var)	
#define new_s(type, var)     new(type);     new__chk(var)
#define newc_s(type, var, c) newc(type, c); new__chk(var) 

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
	return isdigit(c) || c == '.';
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