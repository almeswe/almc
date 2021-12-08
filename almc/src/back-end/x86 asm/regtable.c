#include "regtable.h"

#define STATE_TO_ALL_COMPOUNDED(regstr, state)			               \
	for (int i = 0; i < REGISTER_ENUMERATOR_SCALAR; i++) \
		table->reg_table[regstr+i] = state;

const char* registers_str[] = {
	// 32bit
	[EAX] = "eax",
	[EBX] = "ebx",
	[ECX] = "ecx",
	[EDX] = "edx",
	[ESP] = "esp",
	[EBP] = "ebp",
	[ESI] = "esi",
	[EDI] = "edi",
	// 16bit 
	[AX] = "ax",
	[BX] = "bx",
	[CX] = "cx",
	[DX] = "dx",
	[SP] = "sp",
	[BP] = "bp",
	[SI] = "si",
	[DI] = "di",
	// lower 8bit
	[AL] = "al",
	[BL] = "bl",
	[CL] = "cl",
	[DL] = "dl",
	[SPL] = "spl",
	[BPL] = "bpl",
	[SIL] = "sil",
	[DIL] = "dil"
};

RegisterTable* regtable_new()
{
	RegisterTable* regtable = new_s(RegisterTable, regtable);
	for (int i = 0; i < REGISTERS_COUNT; i++)
		regtable->reg_table[i] = REGISTER_FREE;
	return regtable->ret_reg = -1, regtable;
}

const char* get_register_str(int reg)
{
	return (reg < 0 || reg > REGISTERS_COUNT) ?
		NULL : registers_str[reg];
}

int get_unreserved_register(RegisterTable* table, RegisterSize size)
{
	// this value would map to the needed register size depending on size
	int offset = -(size / 8 - 3);

	for (int i = offset; i < REGISTERS_COUNT; i += REGISTER_ENUMERATOR_SCALAR)
		if (table->reg_table[i] == REGISTER_FREE)
			return reserve_register(table, i), i;

	// case when all common registers are reserved (at least)
	for (int i = offset; i < REGISTERS_COUNT; i += REGISTER_ENUMERATOR_SCALAR)
		if (table->reg_table[i] == REGISTER_RESERVED)
			return reserve_register(table, i), i;

	return -1;
}

int reserve_register(RegisterTable* table, int reg)
{
	reg = reg - (reg % REGISTER_ENUMERATOR_SCALAR);

	if (reg < 0 || reg > REGISTERS_COUNT)
		return 0;
	else
	{ 
		switch (table->reg_table[reg])
		{
		case REGISTER_FREE:
			STATE_TO_ALL_COMPOUNDED(reg, REGISTER_RESERVED);
			break;
		case REGISTER_RESERVED:
			PUSH32(get_register_str(reg));
			STATE_TO_ALL_COMPOUNDED(reg, REGISTER_NEEDS_RESTORE);
			break;
		case REGISTER_NEEDS_RESTORE:
			return 0;
		}
		return 1;
	}
}

int unreserve_register(RegisterTable* table, int reg)
{
	reg = reg - (reg % REGISTER_ENUMERATOR_SCALAR);

	if (reg < 0 || reg > REGISTERS_COUNT)
		return 0;
	else
	{
		switch (table->reg_table[reg])
		{
		case REGISTER_RESERVED:
			STATE_TO_ALL_COMPOUNDED(reg, REGISTER_FREE);
			break;
		case REGISTER_NEEDS_RESTORE:
			POP32(get_register_str(reg));
			STATE_TO_ALL_COMPOUNDED(reg, REGISTER_RESERVED);
			break;
		}
		return 1;
	}
}