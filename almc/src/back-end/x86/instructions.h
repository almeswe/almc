#ifndef ALMC_BACK_END_x86_INSTRUCTIONS
#define ALMC_BACK_END_x86_INSTRUCTIONS

#include <string.h>
#include "..\..\utils\common.h"

#define ADC     0
#define ADD     1
#define AND     2
#define CALL    3
#define CBW     4
#define CMP     5
#define DIV     6
#define IDIV    7
#define IMUL    8
#define JCXZ    9
#define JMP     10
#define JE      11
#define JNE     12
#define JG      13
#define JGE     14
#define JA      15
#define JAE     16
#define JL      17
#define JLE     18
#define JB      19
#define JBE     20
#define JO      21
#define JNO     22
#define JZ      23
#define JNZ     24
#define JS      25
#define JNS     26
#define LEA     27
#define LODSB   28
#define LODSW   29
#define LOOP    30
#define MOV     31
#define MOVSB   32
#define MOVSW   33
#define MUL     34
#define NEG     35
#define NOP     36
#define NOT     37
#define OR      38
#define POP     39
#define PUSH    40
#define PUSHA   41
#define POPA    42
#define RCL     43
#define RCR     44
#define RET     45
#define ROL     46
#define ROR     47
#define SAL     48
#define SAR     49
#define SHL     50
#define SHR     51
#define STOSB   52
#define STOSW   53
#define SUB     54
#define TEST    55
#define XOR     56
#define XCHG    57

#define _LABEL 0x100

static char* instructions_str[] = {
	[ADC] = "adc",
	[ADD] = "add",
	[AND] = "and",
	[CALL] = "call",
	[CBW] = "cbw",
	[CMP] = "cmp",
	[DIV] = "div",
	[IDIV] = "idiv",
	[IMUL] = "imul",
	[JCXZ] = "jcxz",
	[JMP] = "jmp",
	[JE] = "je",
	[JNE] = "jne",
	[JG] = "jg",
	[JGE] = "jge",
	[JA] = "ja",
	[JAE] = "jae",
	[JL] = "jl",
	[JLE] = "jle",
	[JB] = "jb",
	[JBE] = "jbe",
	[JO] = "jo",
	[JNO] = "jno",
	[JZ] = "jz",
	[JNZ] = "jnz",
	[JS] = "js",
	[JNS] = "jns",
	[LEA] = "lea",
	[LODSB] = "lodsb",
	[LODSW] = "lodsw",
	[LOOP] = "loop",
	[MOV] = "mov",
	[MOVSB] = "movsb",
	[MOVSW] = "movsw",
	[MUL] = "mul",
	[NEG] = "neg",
	[NOP] = "nop",
	[NOT] = "not",
	[OR] = "or",
	[POP] = "pop",
	[PUSH] = "push",
	[PUSHA] = "pusha",
	[POPA] = "popa",
	[RCL] = "rcl",
	[RCR] = "rcr",
	[RET] = "ret",
	[ROL] = "rol",
	[ROR] = "ror",
	[SAL] = "sal",
	[SAR] = "sar",
	[SHL] = "shl",
	[SHR] = "shr",
	[STOSB] = "stosb",
	[STOSW] = "stosw",
	[SUB] = "sub",
	[TEST] = "test",
	[XOR] = "xor",
	[XCHG] = "xchg"
};

inline const char* instr_tostr(unsigned int instr)
{
	return instructions_str[instr];
}

#define OUT(code) \
	printf("\t %s \n", code)

#define PUSH32(op)   \
	OUT(frmt("push %s", op))

#define POP32(to_op) \
	OUT(frmt("pop  %s", to_op))

#define MOV32(to, from) \
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

#define ADD32(to, from) \
	OUT(frmt("add  %s, %s", to, from))

#define SUB32(to, from) \
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