#ifndef ALMC_BACK_END_x86_INSTRUCTIONS
#define ALMC_BACK_END_x86_INSTRUCTIONS

#include <string.h>
#include "..\..\utils\common.h"

#define OUT(code) \
	printf("\t %s \n", code)

#define PUSH(op)   \
	OUT(frmt("push %s", op))

#define POP(to_op) \
	OUT(frmt("pop  %s", to_op))

#define MOV(to, from) \
	OUT(frmt("mov  %s, %s", to, from))

#define OR32(to, from) \
	OUT(frmt("or   %s, %s", to, from))

#define XOR32(to, from) \
	OUT(frmt("xor  %s, %s", to, from))

#define AND32(to, from) \
	OUT(frmt("and  %s, %s", to, from))

#define	NOT32(to, from) \
	OUT(frmt("not  %s, %s", to, from))

#define NEG32(to) \
	OUT(frmt("neg  %s", to));

#define NOT32(to) \
	OUT(frmt("not  %s", to));

#define INC32(to) \
	OUT(frmt("inc  %s", to));

#define DEC32(to) \
	OUT(frmt("dec  %s", to));

#define ADD(to, from) \
	OUT(frmt("add  %s, %s", to, from))

#define SUB(to, from) \
	OUT(frmt("sub  %s, %s", to, from))

#define MUL32(to, from)             \
	OUT(frmt("mul  %s", from))

#define MOD32(to, from)             \
	XOR32("edx", "edx");			\
	OUT(frmt("div  %s", from))     			

#define SHL32(to, by) \
	OUT(frmt("shl  %s, %s", to, by))     			

#define SHR32(to, by) \
	OUT(frmt("shr  %s, %s", to, by))     			

#endif 