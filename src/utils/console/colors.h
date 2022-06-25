#ifndef _ALMC_CONSOLE_COLOR_HANDLES_H
#define _ALMC_CONSOLE_COLOR_HANDLES_H

#ifdef 0

	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
	#undef  WIN32_LEAN_AND_MEAN

	// ALMC_CONSOLE prefix added to avoid name conflicts with standart library

	#define ALMC_CONSOLE_BLUE			0x09
	#define ALMC_CONSOLE_GREEN			0x0A
	#define ALMC_CONSOLE_CYAN			0x0B
	#define ALMC_CONSOLE_RED			0x0C
	#define ALMC_CONSOLE_MAGENTA		0x0D
	#define ALMC_CONSOLE_YELLOW			0x0E
	#define ALMC_CONSOLE_WHITE			0x0F
	#define ALMC_CONSOLE_GRAY			0x07

	#define ALMC_CONSOLE_DARKBLUE		0x01
	#define ALMC_CONSOLE_DARKGREEN		0x02
	#define ALMC_CONSOLE_DARKCYAN		0x03
	#define ALMC_CONSOLE_DARKRED		0x04
	#define ALMC_CONSOLE_DARKMAGENTA	0x05
	#define ALMC_CONSOLE_DARKYELLOW		0x06
	#define ALMC_CONSOLE_DARKGRAY		0x08

	#define setdefaultc										\
		SetConsoleTextAttribute(							\
			GetStdHandle(-11), ALMC_CONSOLE_GRAY)

	#define printfc(color, format, ...)						\
		SetConsoleTextAttribute(							\
			GetStdHandle(-11), color),						\
				printf(format, __VA_ARGS__), setdefaultc

#endif

#endif // _ALMC_CONSOLE_COLOR_HANDLES_H