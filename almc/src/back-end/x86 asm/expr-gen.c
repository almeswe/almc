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
	((expr->kind == EXPR_CONST) || \
	 (expr->kind == EXPR_IDNT))

void gen_expr32(Expr* expr, StackFrame* frame)
{
	if (is_const_expr(expr, TABLE))
	{
		reserve_register(REGISTERS, EAX),
			PROC_CODE_LINE2(MOV, get_register_str(EAX),
				frmt("%d", evaluate_expr_itype(expr)));
	}
	else
	{
		switch (expr->kind)
		{
		case EXPR_IDNT:
		case EXPR_CONST:
			reserve_register(REGISTERS, EAX);
			gen_primary_expr32(expr, EAX, frame);
			break;
		/*case EXPR_UNARY_EXPR:
			gen_unary_expr2(expr->unary_expr, frame);
			break;*/
		case EXPR_BINARY_EXPR:
			gen_binary_expr2(expr->binary_expr, frame);
			break;
		/*case EXPR_FUNC_CALL:
			gen_func_call(expr->func_call, frame);
			break;*/
		default:
			assert(0);
		}
	}
}

void gen_idnt32(Idnt* idnt, StackFrame* frame)
{
	//todo: check for enum idnt
	StackFrameEntity* entity = 
		get_entity_by_name(idnt->svalue, frame);
	assert(entity);
	assert(!IS_AGGREGATE_TYPE(idnt->type));

	assert(idnt->type);

	char* arg1 = get_register_str(
		get_part_of_reg(EAX, idnt->type->size * 8));
	char* arg2 = frmt("%s ptr %s[ebp]", 
		get_ptr_prefix(idnt->type), entity->definition);

	PROC_CODE_LINE2(MOV, arg1, arg2);
}

void gen_primary_expr32(Expr* prim_expr, int reg, StackFrame* frame)
{
	assert(reg >= EAX && reg <= ESI);

	const char* to = get_register_str(reg);
	switch (prim_expr->kind)
	{
	case EXPR_CONST:
		switch (prim_expr->cnst->kind)
		{
		case CONST_INT:
			PROC_CODE_LINE2(MOV, to,
				frmt("%d", prim_expr->cnst->ivalue));
			break;
		case CONST_UINT:
			PROC_CODE_LINE2(MOV, to,
				frmt("%i", prim_expr->cnst->uvalue));
			break;
		default:
			break;
		}
		break;
	case EXPR_IDNT:
		gen_idnt32(prim_expr->idnt, frame);
		break;
	default:
		assert(0);
	}
}
void gen_unary_expr2(UnaryExpr* unary_expr, StackFrame* frame)
{
	const char* target = get_register_str(EAX);

	if (!IS_PRIMARY_EXPR(unary_expr->expr))
		gen_expr32(unary_expr->expr, frame);
	else
	{
		reserve_register(frame->regtable, EAX);
		gen_primary_expr32(unary_expr->expr, EAX, frame);
	}

	switch (unary_expr->kind)
	{
	/*case UNARY_PLUS:
		break;
	case UNARY_MINUS:
		NEG32(target);
		break;
	case UNARY_BW_NOT:
		NOT32(target);
		break;*/
	default:
		assert(0);
	}
}

typedef struct 
{
	Type* type;
	int32_t offset;
	StackFrameEntity* entity;
	bool stored_in_accumulator;
} _accessor_data;

_accessor_data* gen_accessor_data(BinaryExpr* expr, StackFrame* frame)
{
	_accessor_data* data = NULL;
	if (expr->lexpr->kind != EXPR_IDNT)
		data = gen_accessor_data(expr->lexpr->binary_expr, frame);
	else
	{
		data = cnew_s(_accessor_data, data, 1);
		data->type = expr->lexpr->idnt->type;
		data->stored_in_accumulator = false;
		data->entity = get_entity_by_name(
			expr->lexpr->idnt->svalue, frame);
	}

	if (expr->kind == BINARY_PTR_MEMBER_ACCESSOR)
	{
		char* arg1 = get_register_str(EAX), arg2 = NULL;
		if (!data->stored_in_accumulator)
		{
			data->stored_in_accumulator = true;
			arg2 = frmt("%s ptr %s[ebp+%d]", get_ptr_prefix(data->type),
				data->entity->definition, data->offset);
		}
		else
			arg2 = frmt("%s ptr [eax+%d]", 
				get_ptr_prefix(data->type), data->offset);
		data->offset = 0;
		PROC_CODE_LINE2(LEA, arg1, arg2);
	}

	data->type = get_base_type(data->type);
	assert(IS_STRUCT_OR_UNION_TYPE(data->type));
	char* member_name = expr->rexpr->idnt->svalue;
	for (size_t i = 0; i < sbuffer_len(data->type->members); i++)
		if (strcmp(data->type->members[i]->name, member_name) == 0)
			return data->offset += data->type->members[i]->offset,
				data->type = data->type->members[i]->type, data;
	assert(0);
}

void gen_binary_accessor_expr32(BinaryExpr* expr, StackFrame* frame)
{
	_accessor_data* data = gen_accessor_data(expr, frame);

	assert(data->type);
	assert(!IS_AGGREGATE_TYPE(data->type));

	char* arg1 = get_register_str(
		get_part_of_reg(EAX, data->type->size * 8));
	char* arg2 = NULL;
	if (!data->stored_in_accumulator)
	{
		arg2 = frmt("%s ptr %s[ebp+%d]", get_ptr_prefix(data->type),
			data->entity->definition, data->offset);
	}
	else
	{
		arg2 = frmt("%s ptr [eax+%d]", 
			get_ptr_prefix(data->type), data->offset);
	}
	PROC_CODE_LINE2(MOV, arg1, arg2);
	free(data);
}

void gen_binary_expr2(BinaryExpr* binary_expr, StackFrame* frame)
{
	switch (binary_expr->kind)
	{
	case BINARY_MEMBER_ACCESSOR:
	case BINARY_PTR_MEMBER_ACCESSOR:
	case BINARY_ARR_MEMBER_ACCESSOR:
		return gen_binary_accessor_expr32(binary_expr, frame);
	}

	#define RESERVE_TEMP_REG  \
		temp_reg = get_unreserved_register(frame->regtable, REGSIZE_DWORD)

	#define GEN_ASSIGN_EXPR(action)           

/*
		action;                               \
		to = gen_idnt_addr(                   \
			binary_expr->lexpr->idnt, frame); \
		MOV32(to, get_register_str(EAX));     \
		free(to)
*/

	int temp_reg;

	if (!IS_PRIMARY_EXPR(binary_expr->lexpr) &&
		IS_PRIMARY_EXPR(binary_expr->rexpr))
	{
		gen_expr32(binary_expr->lexpr, frame);
		RESERVE_TEMP_REG;
		gen_primary_expr32(binary_expr->rexpr, temp_reg, frame);
	}
	else if (IS_PRIMARY_EXPR(binary_expr->lexpr) &&
		IS_PRIMARY_EXPR(binary_expr->rexpr))
	{
		reserve_register(frame->regtable, EAX);
		gen_primary_expr32(binary_expr->lexpr, EAX, frame);
		RESERVE_TEMP_REG;
		gen_primary_expr32(binary_expr->rexpr, temp_reg, frame);
	}
	else if (IS_PRIMARY_EXPR(binary_expr->lexpr) &&
		!IS_PRIMARY_EXPR(binary_expr->rexpr))
	{
		gen_expr32(binary_expr->rexpr, frame);
		RESERVE_TEMP_REG;
		gen_primary_expr32(binary_expr->lexpr, temp_reg, frame);
		// 1+(2-(3+(4-5)))
		// ((1+2*3-4)-(5+6*7))*5
		// (1+2-3*(4-5+6-(7*8+9-(10-11+12*13))))
	}
	else
	{
		gen_expr32(binary_expr->lexpr, frame);
		PUSH32(get_register_str(EAX));
		unreserve_register(frame->regtable, EAX);
		gen_expr32(binary_expr->rexpr, frame);
		RESERVE_TEMP_REG;
		MOV32(get_register_str(temp_reg), get_register_str(EAX));
		POP32(get_register_str(EAX));
	}

	char* to = get_register_str(EAX);
	char* from = get_register_str(temp_reg);
	
	switch (binary_expr->kind)
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
	for (int i = sbuffer_len(func_call->args) - 1; i >= 0; i--)
	{
		gen_expr32(func_call->args[i], frame);
		unreserve_register(frame->regtable, EAX);
		PUSH32(get_register_str(EAX));
	}
	OUT(frmt("call %s", func_call->name));
}