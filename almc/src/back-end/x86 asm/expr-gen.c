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

#define IS_PRIMARY_EXPR(expr)      \
	((expr->type == EXPR_CONST) || \
	 (expr->type == EXPR_IDNT))

void gen_expr2(Expr* expr, StackFrame* frame)
{
	switch (expr->type)
	{
	case EXPR_IDNT:
	case EXPR_CONST:
		reserve_register(frame->regtable, EAX);
		gen_primary_expr2(expr, EAX, frame);
		break;
	case EXPR_UNARY_EXPR:
		gen_unary_expr2(expr->unary_expr, frame);
		break;
	case EXPR_BINARY_EXPR:
		gen_binary_expr2(expr->binary_expr, frame);
		break;
	case EXPR_FUNC_CALL:
		gen_func_call(expr->func_call, frame);
		break;
	default:
		assert(0);
	}
}


char* gen_idnt_addr(Idnt* idnt, StackFrame* frame)
{
	int index, size, prefix;

	if ((index = get_local_by_name(idnt->svalue, frame)) >= 0)
	{
		VarDecl* var_decl = frame->locals[index];
		size = get_type_size(var_decl->type_var->type);
		prefix = get_type_prefix(var_decl->type_var->type);
		return frmt("%s PTR [ebp-%d]", get_predefined_type_str(prefix),
			frame->local_offsets[index]);
	}
	else if ((index = get_argument_by_name(idnt->svalue, frame)) >= 0)
	{
		TypeVar* type_var = frame->arguments[index];
		size = get_type_size(type_var->type);
		prefix = get_type_prefix(type_var->type);
		return frmt("%s PTR [ebp+%d]", get_predefined_type_str(prefix),
			frame->argument_offsets[index]);
	}
	else
		assert(0);
}

void gen_primary_expr2(Expr* prim_expr, int reg, StackFrame* frame)
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
			MOV32(to, frmt("%d", prim_expr->cnst->ivalue));
			break;
		default:
			break;
		}
		break;
	case EXPR_IDNT:
		MOV32(to, gen_idnt_addr(prim_expr->idnt, frame));
		break;
	default:
		assert(0);
	}
}
void gen_unary_expr2(UnaryExpr* unary_expr, StackFrame* frame)
{
	const char* target = get_register_str(EAX);

	if (!IS_PRIMARY_EXPR(unary_expr->expr))
		gen_expr2(unary_expr->expr, frame);
	else
	{
		reserve_register(frame->regtable, EAX);
		gen_primary_expr2(unary_expr->expr, EAX, frame);
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
void gen_binary_expr2(BinaryExpr* binary_expr, StackFrame* frame)
{
	#define RESERVE_TEMP_REG  \
		temp_reg = get_unreserved_register(frame->regtable)

	#define GEN_ASSIGN_EXPR(action)           \
		action;                               \
		to = gen_idnt_addr(                   \
			binary_expr->lexpr->idnt, frame); \
		MOV32(to, get_register_str(EAX));     \
		free(to)

	int temp_reg;

	if (!IS_PRIMARY_EXPR(binary_expr->lexpr) &&
		IS_PRIMARY_EXPR(binary_expr->rexpr))
	{
		gen_expr2(binary_expr->lexpr, frame);
		RESERVE_TEMP_REG;
		gen_primary_expr2(binary_expr->rexpr, temp_reg, frame);
	}
	else if (IS_PRIMARY_EXPR(binary_expr->lexpr) &&
		IS_PRIMARY_EXPR(binary_expr->rexpr))
	{
		reserve_register(frame->regtable, EAX);
		gen_primary_expr2(binary_expr->lexpr, EAX, frame);
		RESERVE_TEMP_REG;
		gen_primary_expr2(binary_expr->rexpr, temp_reg, frame);
	}
	else if (IS_PRIMARY_EXPR(binary_expr->lexpr) &&
		!IS_PRIMARY_EXPR(binary_expr->rexpr))
	{
		gen_expr2(binary_expr->rexpr, frame);
		RESERVE_TEMP_REG;
		gen_primary_expr2(binary_expr->lexpr, temp_reg, frame);
		// 1+(2-(3+(4-5)))
		// ((1+2*3-4)-(5+6*7))*5
		// (1+2-3*(4-5+6-(7*8+9-(10-11+12*13))))
	}
	else
	{
		gen_expr2(binary_expr->lexpr, frame);
		PUSH32(get_register_str(EAX));
		unreserve_register(frame->regtable, EAX);
		gen_expr2(binary_expr->rexpr, frame);
		RESERVE_TEMP_REG;
		MOV32(get_register_str(temp_reg), get_register_str(EAX));
		POP32(get_register_str(EAX));
	}

	char* to = get_register_str(EAX);
	char* from = get_register_str(temp_reg);
	
	switch (binary_expr->type)
	{
	case BINARY_ASSIGN:
		GEN_ASSIGN_EXPR(MOV32(to, from));
		break;
	case BINARY_ADD:
		ADD32(to, from);
		break;
	case BINARY_ADD_ASSIGN:
		GEN_ASSIGN_EXPR(ADD32(to, from));
		break;
	case BINARY_SUB:
		SUB32(to, from);
		break;
	case BINARY_SUB_ASSIGN:
		GEN_ASSIGN_EXPR(SUB32(to, from));
		break;
	case BINARY_MOD:
		reserve_register(frame->regtable, EDX);
		MOD32(to, from);
		MOV32(to, get_register_str(EDX));
		unreserve_register(frame->regtable, EDX);
		break;
	case BINARY_MOD_ASSIGN:
		reserve_register(frame->regtable, EDX);
		GEN_ASSIGN_EXPR(MOD32(to, from));
		MOV32(to, get_register_str(EDX));
		unreserve_register(frame->regtable, EDX);
		break;
	case BINARY_MULT:
		reserve_register(frame->regtable, EDX);
		MUL32(to, from);
		unreserve_register(frame->regtable, EDX);
		break;
	case BINARY_MUL_ASSIGN:
		reserve_register(frame->regtable, EDX);
		GEN_ASSIGN_EXPR(MUL32(to, from));
		unreserve_register(frame->regtable, EDX);
		break;
	case BINARY_LSHIFT:
		SHL32(to, from);
		break;
	case BINARY_LSHIFT_ASSIGN:
		GEN_ASSIGN_EXPR(SHL32(to, from));
		break;
	case BINARY_RSHIFT:
		SHR32(to, from);
		break;
	case BINARY_RSHIFT_ASSIGN:
		GEN_ASSIGN_EXPR(SHR32(to, from));
		break;
	case BINARY_BW_OR:
		OR32(to, from);
		break;
	case BINARY_BW_OR_ASSIGN:
		GEN_ASSIGN_EXPR(OR32(to, from));
		break;
	case BINARY_BW_AND:
		AND32(to, from);
		break;
	case BINARY_BW_AND_ASSIGN:
		GEN_ASSIGN_EXPR(AND32(to, from));
		break;
	case BINARY_BW_XOR:
		XOR32(to, from);
		break;
	case BINARY_BW_XOR_ASSIGN:
		GEN_ASSIGN_EXPR(XOR32(to, from));
		break;
	default:
		assert(0);
	}
	unreserve_register(frame->regtable, temp_reg);

#undef GEN_ASSIGN_EXPR
#undef RESERVE_TEMP_REG
}

void gen_func_call(FuncCall* func_call, StackFrame* frame)
{
	for (int i = sbuffer_len(func_call->func_args) - 1; i >= 0; i--)
	{
		gen_expr2(func_call->func_args[i], frame);
		unreserve_register(frame->regtable, EAX);
		PUSH32(get_register_str(EAX));
	}
	OUT(frmt("call %s", func_call->func_name));
}