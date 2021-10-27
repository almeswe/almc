#ifndef ALMC_BACK_END_x86_REGISTERS
#define ALMC_BACK_END_x86_REGISTERS

#define EAX 0
#define EBX 1
#define ECX 2
#define EDX 3
#define ESP 4
#define EBP 5
#define ESI 6

#define REGISTER_FREE		   0xD
#define REGISTER_RESERVED	   0xE
#define REGISTER_NEEDS_RESTORE 0xF

#include "instructions.h"

/*typedef enum x86_ResultSize
{
	BYTE = 8,
	WORD = 16,
	DWORD = 32,
} ResultSize;*/

typedef struct x86_RegisterTable
{
	//todo: extend for minors
	int ret_reg;
	int reg_table[6];
} RegisterTable;

RegisterTable* regtable_new();

const char* get_register_str(int reg);
int get_unreserved_register(RegisterTable* table);
int reserve_register(RegisterTable* table, int reg);
int unreserve_register(RegisterTable* table, int reg);

#endif