#ifndef _ALMC_BACK_END_x86_REGISTERS_H
#define _ALMC_BACK_END_x86_REGISTERS_H

#include "instructions.h"
#include "..\..\utils\common.h"

// constant values for checking the ability of register to be accessed
#define REGISTER_FREE		   0xF
#define REGISTER_RESERVED	   0xA
#define REGISTER_NEEDS_RESTORE 0xB

#define	REGISTER8_ENUMERATING_OFFSET  0x2
#define REGISTER16_ENUMERATING_OFFSET 0x1
#define REGISTER32_ENUMERATING_OFFSET 0x0

// all registers count	
#define SSE_REGISTERS_COUNT (XMM7 - XMM0 + 1)
#define DEFAULT_REGISTERS_COUNT (DIL - EAX + 1)
#define GENERAL_REGISTERS_COUNT (DL + 1)

// used to for enumerating through the all register of the same size
// just multiply on current index (not for sse registers)
#define REGISTER_ENUMERATOR_SCALAR 0x3

#define is_gp_reg(reg) \
	((reg) >= EAX && (reg) <= DL)

#define is_sse_reg(reg) \
	((reg) >= XMM0 && (reg) <= XMM7)

#include <assert.h>

#define report_regmanage_bug(msg, ...) \
	printf(msg, __VA_ARGS__), assert(0)

enum x86_Registers
{
	EAX, AX, AL,
	EBX, BX, BL,
	ECX, CX, CL,
	EDX, DX, DL,
	ESP, SP, SPL,
	EBP, BP, BPL,
	ESI, SI, SIL,
	EDI, DI, DIL
};

enum x86_SSE_Registers
{
	XMM0 = DIL + 1, XMM1,
	XMM2, XMM3, XMM4,
	XMM5, XMM6, XMM7,
};

typedef enum x86_RegisterSize
{
	REGSIZE_BYTE  = 8,
	REGSIZE_WORD  = 16,
	REGSIZE_DWORD = 32,
	REGSIZE_PACKED = 128
} RegisterSize;

typedef struct x86_RegisterTable
{
	int reg_table[DEFAULT_REGISTERS_COUNT + SSE_REGISTERS_COUNT];
} RegisterTable;

RegisterTable* regtable_new();

char* get_register_str(int reg);
int reserve_register(RegisterTable* table, int reg);
int reserve_sse_register(RegisterTable* table, int sse_reg);
bool unreserve_register(RegisterTable* table, int reg);
bool unreserve_sse_register(RegisterTable* table, int sse_reg);
int get_unreserved_register(RegisterTable* table, RegisterSize size);
int get_unreserved_sse_register(RegisterTable* table);
int get_subregister(int reg, RegisterSize size);

#endif // _ALMC_BACK_END_x86_REGISTERS_H