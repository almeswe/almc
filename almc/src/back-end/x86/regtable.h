#ifndef ALMC_BACK_END_x86_REGISTERS
#define ALMC_BACK_END_x86_REGISTERS

#define EAX	0x00
#define AX	0x01
#define AL	0x02
#define EBX	0x03
#define BX	0x04
#define BL	0x05
#define ECX	0x06
#define CX	0x07
#define CL	0x08
#define EDX	0x09
#define DX	0x0A
#define DL	0x0B
#define ESP	0x0C
#define SP	0x0D
#define SPL	0x0E
#define EBP	0x0F
#define BP	0x10
#define BPL	0x11
#define ESI	0x12
#define SI	0x13
#define SIL	0x14
#define EDI	0x15
#define DI	0x16
#define DIL	0x17

// constant values for checking the ability of register to be accessed
#define REGISTER_FREE		   0xF
#define REGISTER_RESERVED	   0xA
#define REGISTER_NEEDS_RESTORE 0xB

// all registers count	
#define REGISTERS_COUNT	0x18

#define	REGISTER8_ENUMERATING_OFFSET  0x2
#define REGISTER16_ENUMERATING_OFFSET 0x1
#define REGISTER32_ENUMERATING_OFFSET 0x0

// used to for enumerating through the all register of the same size
// just multiply on current index
#define REGISTER_ENUMERATOR_SCALAR 0x3

#include "instructions.h"

typedef enum x86_RegisterSize
{
	REGSIZE_BYTE  = 8,
	REGSIZE_WORD  = 16,
	REGSIZE_DWORD = 32,
} RegisterSize;

typedef struct x86_RegisterTable
{
	int ret_reg;
	int reg_table[REGISTERS_COUNT];
} RegisterTable;

RegisterTable* regtable_new();

const char* get_register_str(int reg);
int reserve_register(RegisterTable* table, int reg);
int unreserve_register(RegisterTable* table, int reg);
int get_unreserved_register(RegisterTable* table, RegisterSize size);
int get_part_of_reg(int reg, RegisterSize size);

#endif