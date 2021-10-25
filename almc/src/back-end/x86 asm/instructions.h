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

#define ADD(to, from) \
	OUT(frmt("add  %s, %s", to, from))

#define XOR(to, from) \
	OUT(frmt("xor  %s, %s", to, from))

#define SUB(to, from) \
	OUT(frmt("sub  %s, %s", to, from))

#define MUL32(to, from)             \
	OUT(frmt("mul  %s", from))

#define MOD32(to, from)             \
	XOR("edx", "edx");				\
	OUT(frmt("div  %s", from))     			

#endif 