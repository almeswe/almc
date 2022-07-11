#ifndef _ALMC_COLOR_HANDLES_H
#define _ALMC_COLOR_HANDLES_H

#define _c(c, ...) c __VA_ARGS__ RESET 

#ifdef _WIN32
	#error "this module cannot be compiled on this platform" 
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
	#undef  WIN32_LEAN_AND_MEAN

	#define BLUE		0x09
	#define GREEN		0x0A
	#define CYAN		0x0B
	#define RED			0x0C
	#define MAGENTA		0x0D
	#define YELLOW		0x0E
	#define WHITE		0x0F
	#define GRAY		0x07

	#define DARKBLUE	0x01
	#define DARKGREEN	0x02
	#define DARKCYAN	0x03
	#define DARKRED		0x04
	#define DARKMAGENTA	0x05
	#define DARKYELLOW	0x06
	#define DARKGRAY	0x08

	#define setdefaultc										\
		SetConsoleTextAttribute(							\
			GetStdHandle(-11), GRAY)

	#define printfc(color, format, ...)						\
		SetConsoleTextAttribute(							\
			GetStdHandle(-11), color),						\
				printf(format, __VA_ARGS__), setdefaultc
#elif __linux__
	#include <stdio.h>

	#define BLK "\e[0;30m"
	#define RED "\e[0;31m"
	#define GRN "\e[0;32m"
	#define YEL "\e[0;33m"
	#define BLU "\e[0;34m"
	#define MAG "\e[0;35m"
	#define CYN "\e[0;36m"
	#define WHT "\e[0;37m"

	#define BBLK "\e[1;30m"
	#define BRED "\e[1;31m"
	#define BGRN "\e[1;32m"
	#define BYEL "\e[1;33m"
	#define BBLU "\e[1;34m"
	#define BMAG "\e[1;35m"
	#define BCYN "\e[1;36m"
	#define BWHT "\e[1;37m"

	#define UBLK "\e[4;30m"
	#define URED "\e[4;31m"
	#define UGRN "\e[4;32m"
	#define UYEL "\e[4;33m"
	#define UBLU "\e[4;34m"
	#define UMAG "\e[4;35m"
	#define UCYN "\e[4;36m"
	#define UWHT "\e[4;37m"

	#define BLKB "\e[40m"
	#define REDB "\e[41m"
	#define GRNB "\e[42m"
	#define YELB "\e[43m"
	#define BLUB "\e[44m"
	#define MAGB "\e[45m"
	#define CYNB "\e[46m"
	#define WHTB "\e[47m"

	#define BLKHB "\e[0;100m"
	#define REDHB "\e[0;101m"
	#define GRNHB "\e[0;102m"
	#define YELHB "\e[0;103m"
	#define BLUHB "\e[0;104m"
	#define MAGHB "\e[0;105m"
	#define CYNHB "\e[0;106m"
	#define WHTHB "\e[0;107m"

	#define HBLK "\e[0;90m"
	#define HRED "\e[0;91m"
	#define HGRN "\e[0;92m"
	#define HYEL "\e[0;93m"
	#define HBLU "\e[0;94m"
	#define HMAG "\e[0;95m"
	#define HCYN "\e[0;96m"
	#define HWHT "\e[0;97m"

	#define BHBLK "\e[1;90m"
	#define BHRED "\e[1;91m"
	#define BHGRN "\e[1;92m"
	#define BHYEL "\e[1;93m"
	#define BHBLU "\e[1;94m"
	#define BHMAG "\e[1;95m"
	#define BHCYN "\e[1;96m"
	#define BHWHT "\e[1;97m"

	#define RESET "\e[0m"
#endif

#endif // _ALMC_COLOR_HANDLES_H