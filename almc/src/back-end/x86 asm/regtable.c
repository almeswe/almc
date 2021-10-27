#include "regtable.h"

const char* registers_str[7] = {
	"eax",
	"ebx",
	"ecx",
	"edx",
	"esp",
	"ebp",
	"esi"
};

RegisterTable* regtable_new()
{
	RegisterTable* regtable = new_s(RegisterTable, regtable);
	for (int i = 0; i < ESI; i++)
		regtable->reg_table[i] = REGISTER_FREE;
	//regtable->reg_table[EDX] = REGISTER_RESERVED;
	regtable->ret_reg = -1;
	return regtable;
}

const char* get_register_str(int reg)
{
	return (reg < EAX || reg > ESI) ?
		NULL : registers_str[reg];
}

int get_unreserved_register(RegisterTable* table)
{
	for (int i = 0; i < 4; i++)
		if (table->reg_table[i] == REGISTER_FREE)
			return reserve_register(table, i), i;

	// case when all common registers are reserved (at least)
	for (int i = 0; i < 4; i++)
		if (table->reg_table[i] == REGISTER_RESERVED)
			return reserve_register(table, i), i;

	return -1;
}

int reserve_register(RegisterTable* table, int reg)
{
	if (reg < EAX || reg > ESI)
		return 0;
	else
	{
		switch (table->reg_table[reg])
		{
		case REGISTER_FREE:
			table->reg_table[reg] = REGISTER_RESERVED;
			break;
		case REGISTER_RESERVED:
			PUSH(get_register_str(reg));
			table->reg_table[reg] = REGISTER_NEEDS_RESTORE;
			break;
		case REGISTER_NEEDS_RESTORE:
			return 0;
		}
		return 1;
	}
}

int unreserve_register(RegisterTable* table, int reg)
{
	if (reg < EAX || reg > ESI)
		return 0;
	else
	{
		switch (table->reg_table[reg])
		{
		case REGISTER_RESERVED:
			table->reg_table[reg] = REGISTER_FREE;
			break;
		case REGISTER_NEEDS_RESTORE:
			POP(get_register_str(reg));
			table->reg_table[reg] = REGISTER_RESERVED;
			break;
		}
		return 1;
	}
}