#include "gen.h"

#define EAX 0
#define EBX 1
#define ECX 2
#define EDX 3

#define FREE 0
#define RESERVED 1
#define NEEDS_RESTORE 2

#define out(code) printf("\t %s \n", code)

static char* registers_str[] = {
	"eax",
	"ebx",
	"ecx",
	"edx"
};

static int registers[4];
static int result_register;

void init_reserved_registers()
{
	for (int i = 0; i < 4; i++)
		registers[i] = FREE;
	result_register = registers[0];
}

int get_unreserved_register()
{
	for (int i = 0; i < 4; i++)
		if (registers[i] == FREE)
			return reserve_register(i), i;

	// case when all common registers are reserved (at least)
	for (int i = 0; i < 4; i++)
		if (registers[i] == RESERVED)
			return reserve_register(i), i;

	assert(!"No free registers found!");
}

void reserve_register(int reg)
{
	assert(reg <= 3 && reg >= 0);
	switch (registers[reg])
	{
	case FREE:
		registers[reg] = RESERVED;
		break;
	case RESERVED:
		registers[reg] = NEEDS_RESTORE;
		out(frmt("push %s", registers_str[reg]));
		break;
	case NEEDS_RESTORE:
		assert(0);
	}
}

void unreserve_register(int reg)
{
	assert(reg <= 3 && reg >= 0);
	switch (registers[reg])
	{
	case FREE:
		break;
	case RESERVED:
		registers[reg] = FREE;
		break;
	case NEEDS_RESTORE:
		registers[reg] = RESERVED;
		out(frmt("pop  %s", registers_str[reg]));
	}
}

void gen_const(Const* cnst, char* to_reg)
{
	switch (cnst->type)
	{
	case CONST_INT:
	case CONST_UINT:
		out(frmt("mov  %s, %d", to_reg, cnst->ivalue));
		break;
	default:
		assert(0);
	}
}

void gen_unary_expr(Expr* expr)
{
	switch (expr->type)
	{
	default:
		assert(0);
	}
}

/*  
	 1 = 0000 0001
	-1 = 1111 1111

	-1 + 1 = 0

	0000 0001
	1111 1111
	---------
	0000 0000


	-3 + 1 = -2
	2 = 0000 0010
   -2 = 1111 1110

	3 = 0000 0011
   -3 = 1111 1101

   1111 1101
   0000 0001
   ---------
   1111 1110
   1111 1110
   ---------
   1111 1100

   0000 0010

   0000 0010
*/

void gen_binary_operation(int lreg, int rreg, BinaryExprType type)
{
	switch (type)
	{
	case BINARY_ADD:
		out(frmt("add  %s, %s", registers_str[result_register = lreg], registers_str[rreg]));
		break;
	case BINARY_SUB:
		// if signed
		out(frmt("neg  %s", registers_str[rreg]));
		out(frmt("add  %s, %s", registers_str[result_register = lreg], registers_str[rreg]));
		break;
	case BINARY_MULT:
		if (lreg != EAX)
		{
			reserve_register(EAX);
			out(frmt("mov  eax, %s", registers_str[lreg]));
			out(frmt("mul  %s", registers_str[rreg]));
			out(frmt("mov  %s, eax", registers_str[result_register = lreg]));
			unreserve_register(EAX);
		}
		else
			out(frmt("mul  %s", registers_str[result_register = lreg, rreg]));
		break;
	default:
		assert(0);
	}
}

void gen_binary_expr(BinaryExpr* binary)
{
#define is_const(expr) \
	(expr->type == EXPR_CONST)

	int op1 = -1;
	int op2 = -1;

	// case: expr op const
	if (!is_const(binary->lexpr) &&
		is_const(binary->rexpr))
	{
		gen_expr(binary->lexpr);
		op1 = result_register;
		op2 = get_unreserved_register();
		gen_const(binary->rexpr->cnst, registers_str[op2]);
		unreserve_register(op2);
	}
	else if (is_const(binary->lexpr) &&
		is_const(binary->rexpr))
	{
		op1 = get_unreserved_register();
		op2 = get_unreserved_register();
		gen_const(binary->lexpr->cnst, registers_str[op1]);
		gen_const(binary->rexpr->cnst, registers_str[op2]);
		unreserve_register(op2);
	}
	else if (is_const(binary->lexpr) &&
		!is_const(binary->rexpr))
	{
		gen_expr(binary->rexpr);
		op1 = result_register;
		op2 = get_unreserved_register();
		gen_const(binary->lexpr->cnst, registers_str[op2]);
		unreserve_register(op1);
		gen_binary_operation(op2, op1, binary->type);
		return;
	}
	else
	{
		gen_expr(binary->lexpr);
		op1 = result_register;
		gen_expr(binary->rexpr);
		op2 = result_register;
		unreserve_register(op2);
	}
	gen_binary_operation(op1, op2, binary->type);
}

void gen_expr(Expr* expr)
{
	switch (expr->type)
	{
	case EXPR_CONST:
		gen_const(expr->cnst, "eax");
		break;
	//case EXPR_UNARY_EXPR:
	//	gen_unary_expr(expr->binary_expr);
	//	break;
	case EXPR_BINARY_EXPR:
		gen_binary_expr(expr->binary_expr);
		break;
	default:
		assert(0);
	}
}