#include "regtable.h"

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
	[DIL] = "dil",

	//sse simd registers
	[XMM0] = "xmm0",
	[XMM1] = "xmm1",
	[XMM2] = "xmm2",
	[XMM3] = "xmm3",
	[XMM4] = "xmm4",
	[XMM5] = "xmm5",
	[XMM6] = "xmm6",
	[XMM7] = "xmm7"
};

RegisterTable* regtable_new()
{
	RegisterTable* regtable = new_s(RegisterTable, regtable);
	for (int i = EAX; i <= XMM7; i++)
		regtable->reg_table[i] = REGISTER_FREE;
	return regtable;
}

char* get_register_str(int reg)
{
	return (reg < 0 || reg > (sizeof(registers_str) / sizeof(char*))) ?
		NULL : registers_str[reg];
}

int get_unreserved_register(RegisterTable* table, RegisterSize size)
{
	// this value would map to the needed register size depending on size
	int offset = 0;
	switch (size)
	{
	case REGSIZE_BYTE:
		offset = 2;
		break;
	case REGSIZE_WORD:
		offset = 1;
		break;
	case REGSIZE_DWORD:
		offset = 0;
		break;
	case REGSIZE_PACKED:
		return get_unreserved_sse_register(table);
	}
	for (int i = EAX; i <= EDX; i += REGISTER_ENUMERATOR_SCALAR)
		if (table->reg_table[i] == REGISTER_FREE)
			return reserve_register(table, i), i;
	return report_regmanage_bug("bug with register managing met. line: %d, file: %s",
		__LINE__, __FILE__), -1;
}

int get_unreserved_sse_register(RegisterTable* table)
{
	for (int sse_reg = XMM0; sse_reg <= XMM7; sse_reg++)
		if (table->reg_table[sse_reg] == REGISTER_FREE)
			return reserve_sse_register(table, sse_reg);
	return report_regmanage_bug("bug with register managing met. line: %d, file: %s",
		__LINE__, __FILE__), -1;
}

int reserve_register(RegisterTable* table, int reg)
{
	if (is_sse_reg(reg))
		return reserve_sse_register(table, reg);

	// finding the greates register from passed register (reg)
	// needed to set states to all subregisters in future
	reg = reg - (reg % REGISTER_ENUMERATOR_SCALAR);
	if (reg < 0 || reg > DEFAULT_REGISTERS_COUNT)
		return report_regmanage_bug("bug, passed reg is not valid. line: %d, file: %s",
			__LINE__, __FILE__), -1;
	if (table->reg_table[reg] != REGISTER_FREE)
		return report_regmanage_bug("bug with register managing met (reg: %s). line: %d, file: %s",
			get_register_str(reg), __LINE__, __FILE__), -1;
	// binding the state to all subregisters
	for (int i = 0; i < REGISTER_ENUMERATOR_SCALAR; i++)
		table->reg_table[reg+i] = REGISTER_RESERVED;
	return reg;
}

int reserve_sse_register(RegisterTable* table, int sse_reg)
{
	if (!is_sse_reg(sse_reg))
		return report_regmanage_bug("bug, not sse register passed. line: %d, file: %s",
			__LINE__, __FILE__), -1;
	if (table->reg_table[sse_reg] != REGISTER_FREE)
		return report_regmanage_bug("bug with register managing met (reg: %s). line: %d, file: %s",
			get_register_str(sse_reg), __LINE__, __FILE__) , -1;
	return table->reg_table[sse_reg] = REGISTER_RESERVED, sse_reg;
}

bool unreserve_register(RegisterTable* table, int reg)
{
	if (is_sse_reg(reg))
		return unreserve_sse_register(table, reg);

	reg = reg - (reg % REGISTER_ENUMERATOR_SCALAR);
	if (reg < 0 || reg > DEFAULT_REGISTERS_COUNT)
		return report_regmanage_bug("bug, passed reg is not valid. line: %d, file: %s",
			__LINE__, __FILE__), false;
	for (int i = 0; i < REGISTER_ENUMERATOR_SCALAR; i++)
		table->reg_table[reg + i] = REGISTER_FREE;
	return true;
}

bool unreserve_sse_register(RegisterTable* table, int sse_reg)
{
	if (!is_sse_reg(sse_reg))
		return report_regmanage_bug("bug, not sse register passed. line: %d, file: %s",
			__LINE__, __FILE__), false;
	return table->reg_table[sse_reg] = REGISTER_FREE, true;
}

int get_subregister(int reg, RegisterSize size)
{
	switch (size)
	{
	case REGSIZE_BYTE:
		return reg + 2;
	case REGSIZE_WORD:
		return reg + 1;
	}
	return reg;
}