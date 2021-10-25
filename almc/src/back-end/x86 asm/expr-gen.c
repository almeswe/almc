#include "expr-gen.h"

//todo: somehow add support for unsigned/signed arithemetic
//todo: also figure out how to add floating point arithmetic

ExprGenerator* expr_gen_new()
{
	ExprGenerator* expr_gen = new_s(ExprGenerator, expr_gen);
	expr_gen->result = EXPR_GEN_RES_DWORD;
	init_regtable(&expr_gen->regtable);
	return expr_gen;
}

void expr_gen_free(ExprGenerator* expr_gen)
{
	if (expr_gen)
		free(expr_gen);
}

void gen_expr(ExprGenerator* expr_gen, Expr* expr)
{
	switch (expr->type)
	{
	case EXPR_IDNT:
	case EXPR_CONST:
		reserve_register(&expr_gen->regtable, EAX);
		gen_primary_expr(expr_gen, expr, EAX);
		break;
	case EXPR_UNARY_EXPR:
		gen_unary_expr(expr_gen, expr->unary_expr);
		break;
	case EXPR_BINARY_EXPR:
		gen_binary_expr(expr_gen, expr->binary_expr);
		break;
	default:
		assert(0);
	}
}

void gen_primary_expr(ExprGenerator* expr_gen, Expr* prim_expr, int reg)
{
	assert(reg >= EAX && reg <= ESI);
	const char* to = get_register_str(reg);
	switch (prim_expr->type)
	{
	case EXPR_CONST:
		switch (prim_expr->cnst->type)
		{
		case CONST_INT:
		case CONST_UINT:
			MOV(to, frmt("%d", prim_expr->cnst->ivalue));
			break;
		default:
			break;
		}
		break;
	case EXPR_IDNT:
		MOV(to, prim_expr->idnt->svalue);
		break;
	default:
		assert(0);
		//case EXPR_STRING:
		//	break;
	}
}

void gen_unary_expr(ExprGenerator* expr_gen, UnaryExpr* unary_expr)
{
	assert(0);
}

void gen_binary_expr(ExprGenerator* expr_gen, BinaryExpr* binary_expr)
{
#define IS_PRIMARY_EXPR(expr) \
	((expr->type == EXPR_CONST) || \
	(expr->type == EXPR_IDNT))

#define RESERVE_TEMP_REG  \
	temp_reg = get_unreserved_register(&expr_gen->regtable)

	int temp_reg;

	if (!IS_PRIMARY_EXPR(binary_expr->lexpr) &&
		IS_PRIMARY_EXPR(binary_expr->rexpr))
	{
		gen_expr(expr_gen, binary_expr->lexpr);
		RESERVE_TEMP_REG;
		gen_primary_expr(expr_gen, binary_expr->rexpr, temp_reg);
	}
	else if (IS_PRIMARY_EXPR(binary_expr->lexpr) &&
		IS_PRIMARY_EXPR(binary_expr->rexpr))
	{
		reserve_register(&expr_gen->regtable, EAX);
		gen_primary_expr(expr_gen, binary_expr->lexpr, EAX);
		RESERVE_TEMP_REG;
		gen_primary_expr(expr_gen, binary_expr->rexpr, temp_reg);
	}
	else if (IS_PRIMARY_EXPR(binary_expr->lexpr) &&
		!IS_PRIMARY_EXPR(binary_expr->rexpr))
	{
		gen_expr(expr_gen, binary_expr->rexpr);
		RESERVE_TEMP_REG;
		gen_primary_expr(expr_gen, binary_expr->lexpr, temp_reg);
		// 1+(2-(3+(4-5)))
		// ((1+2*3-4)-(5+6*7))*5
		// (1+2-3*(4-5+6-(7*8+9-(10-11+12*13))))
	}
	else
	{
		gen_expr(expr_gen, binary_expr->lexpr);
		PUSH(get_register_str(EAX));
		gen_expr(expr_gen, binary_expr->rexpr);
		RESERVE_TEMP_REG;
		MOV(get_register_str(temp_reg), get_register_str(EAX));
		POP(get_register_str(EAX));
	}

	const char* to = get_register_str(EAX);
	const char* from = get_register_str(temp_reg);

	switch (binary_expr->type)
	{
	case BINARY_ADD:
		ADD(to, from); break;
	case BINARY_SUB:
		SUB(to, from); break;
	case BINARY_MOD:
		reserve_register(&expr_gen->regtable, EDX);
		MOD32(to, from);
		MOV(to, get_register_str(EDX));
		unreserve_register(&expr_gen->regtable, EDX);
		break;
	case BINARY_MULT:
		reserve_register(&expr_gen->regtable, EDX);
		MUL32(to, from); 
		unreserve_register(&expr_gen->regtable, EDX);
		break;
	default:
		assert(0);
	}

	unreserve_register(&expr_gen->regtable, temp_reg);
}