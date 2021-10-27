#include "expr-gen.h"

//todo: somehow add support for unsigned/signed arithemetic
//todo: also figure out how to add floating point arithmetic

/*
	
	!x = 0 (x != 0)
	!x = 1 (x == 0)

	cmp eax, 0
	setne al
	mov eax, al

	setne 

	its can be the integer or float
	this should be placed in ExprGenerator struct (also rename it from x86)
	
	integer - x86
		unsigned/signed
			8, 16, 32, 64
	
	float   - x87
		32 - real4, 64 - real8	
*/

ExprGenerator* expr_gen_new(RegisterTable* regtable)
{
	ExprGenerator* expr_gen = new_s(ExprGenerator, expr_gen);
	expr_gen->result = EXPR_GEN_RES_DWORD;
	expr_gen->regtable = regtable;
	return expr_gen;
}

void expr_gen_free(ExprGenerator* expr_gen)
{
	if (expr_gen)
	{
		free(expr_gen->regtable);
		free(expr_gen);
	}
}

void gen_expr(ExprGenerator* expr_gen, Expr* expr)
{
	switch (expr->type)
	{
	case EXPR_IDNT:
	case EXPR_CONST:
		reserve_register(expr_gen->regtable, EAX);
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

#define IS_PRIMARY_EXPR(expr)      \
	((expr->type == EXPR_CONST) || \
	 (expr->type == EXPR_IDNT))

void gen_unary_expr(ExprGenerator* expr_gen, UnaryExpr* unary_expr)
{
	const char* target = get_register_str(EAX);

	if (!IS_PRIMARY_EXPR(unary_expr->expr))
		gen_expr(expr_gen, unary_expr->expr);
	else
	{
		reserve_register(expr_gen->regtable, EAX);
		gen_primary_expr(expr_gen, unary_expr->expr, EAX);
	}

	switch (unary_expr->type)
	{
	case UNARY_PLUS:
		break;
	case UNARY_MINUS:
		NEG32(target);
		break;
	case UNARY_BW_NOT:
		NOT32(target);
		break;
	}
}

void gen_binary_expr(ExprGenerator* expr_gen, BinaryExpr* binary_expr)
{
#define RESERVE_TEMP_REG  \
	temp_reg = get_unreserved_register(expr_gen->regtable)

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
		reserve_register(expr_gen->regtable, EAX);
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
		unreserve_register(expr_gen->regtable , EAX);
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
		ADD(to, from); 
		break;
	case BINARY_SUB:
		SUB(to, from); 
		break;
	case BINARY_MOD:
		reserve_register(expr_gen->regtable, EDX);
		MOD32(to, from);
		MOV(to, get_register_str(EDX));
		unreserve_register(expr_gen->regtable, EDX);
		break;
	case BINARY_MULT:
		reserve_register(expr_gen->regtable, EDX);
		MUL32(to, from); 
		unreserve_register(expr_gen->regtable, EDX);
		break;
	case BINARY_LSHIFT:
		SHL32(to, from);
		break;
	case BINARY_RSHIFT:
		SHR32(to, from);
		break;
	case BINARY_BW_OR:
		OR32(to, from); 
		break;
	case BINARY_BW_AND:
		AND32(to, from); 
		break;
	case BINARY_BW_XOR:
		XOR32(to, from); 
		break;
	default:
		assert(0);
	}
	unreserve_register(expr_gen->regtable, temp_reg);

#undef RESERVE_TEMP_REG
}