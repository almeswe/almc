#include "expr-gen.h"
#include <math.h>

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
			gen_binary_expr32(expr->binary_expr, frame);
			break;
		case EXPR_FUNC_CALL:
			gen_func_call32(expr->func_call, frame);
			break;
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
		reserve_register(REGISTERS, EAX);
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
	int32_t cap;
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

	char* arg1 = NULL;
	char* arg2 = NULL;
	int temp_reg = 0;
	char* member_name = NULL;

	switch (expr->kind)
	{
	case BINARY_PTR_MEMBER_ACCESSOR:
		// if we met the -> operator,
		// it means that we need to get value stored in address
		// which will vary according stored_in_accumulator flag
		// if itss true, it means that the needed address aleady stored in eax,
		// and no need to calculate offsets based on stack variable, need to calculate it based on eax
		arg1 = get_register_str(EAX);
		if (data->stored_in_accumulator)
			arg2 = frmt("%s ptr [eax+%d]",
				get_ptr_prefix(data->type), data->offset);
		else
		{
			data->stored_in_accumulator = true;
			arg2 = frmt("%s ptr %s[ebp+%d]", get_ptr_prefix(data->type),
				data->entity->definition, data->offset);
		}
		data->offset = 0;
		PROC_CODE_LINE2(LEA, arg1, arg2);
		// getting true type of variable (not pointer)
		// need only one dereference, because -> operator only works
		// with single pointer values by the left side
		data->type = dereference_type(data->type);
		// continuing the process in case BINARY_MEMBER_ACCESSOR

	case BINARY_MEMBER_ACCESSOR:
		assert(IS_STRUCT_TYPE(data->type));
		member_name = expr->rexpr->idnt->svalue;
		for (size_t i = 0; i < sbuffer_len(data->type->members); i++)
			if (strcmp(data->type->members[i]->name, member_name) == 0)
				return data->offset += data->type->members[i]->offset,
					data->type = data->type->members[i]->type, data;
		assert(0);
		break;

	case BINARY_ARR_MEMBER_ACCESSOR:
		assert(0);
		if (data->stored_in_accumulator)
		{
			arg1 = get_register_str(temp_reg = 
				get_unreserved_register(REGISTERS, REGSIZE_DWORD));
			PROC_CODE_LINE2(MOV, arg1, get_register_str(EAX));
		}

		// generate index first
		gen_expr32(expr->rexpr, frame);

		// after calculate the offset based on index and array's base type
		// value of index will stored in eax register
		// reserving free register for multilication
		arg1 = get_register_str(EAX);
		if (data->stored_in_accumulator)
		{
			arg2 = frmt("%s ptr [%s+eax*%d]", get_ptr_prefix(data->type->base),
				get_register_str(temp_reg), data->type->base->size);
			unreserve_register(REGISTERS, temp_reg);
		}
		else
		{
			data->stored_in_accumulator = true;
			arg2 = frmt("%s ptr %s[ebp+eax*%d]", get_ptr_prefix(data->type->base),
				data->entity->definition, data->type->base->size);
		}
		PROC_CODE_LINE2(LEA, arg1, arg2);
		data->offset = 0; // ?
		data->type = dereference_type(data->type);
		break;
	}
	return data;
}

void gen_binary_accessor_expr32(BinaryExpr* expr, StackFrame* frame)
{
	_accessor_data* data = gen_accessor_data(expr, frame);

	assert(data->type);
	assert(!IS_AGGREGATE_TYPE(data->type));

	char* arg1 = get_register_str(
		get_part_of_reg(EAX, data->type->size * 8));
	char* arg2 = NULL;
	// calculating the address that will be relative to bottom of the allocate space
	// this magic -1 is bruted for perfect fitting to the bounds of allocated space
	data->offset = abs(data->offset - 
		(int32_t)data->entity->type->size) - 1;
	if (!data->stored_in_accumulator)
		arg2 = frmt("%s ptr %s[ebp+%d]", get_ptr_prefix(data->type),
			data->entity->definition, data->offset);
	else
		arg2 = frmt("%s ptr [eax+%d]", 
			get_ptr_prefix(data->type), data->offset);
	PROC_CODE_LINE2(MOV, arg1, arg2);
	free(data);
}

void gen_clear_reg(int reg)
{
	PROC_CODE_LINE2(XOR, get_register_str(reg),
		get_register_str(reg));
}

void gen_clear_reg_arg(char* reg_arg)
{
	PROC_CODE_LINE2(XOR, reg_arg, reg_arg);
}

typedef struct {
	// last type which was assigned in last recursion (mostly all functions
	// for determingn the addressible data are recursive).
	// Type needed majorly for specifying the ptr prefix
	Type* type;
	// offset added to address register.
	// will be cleared every time in recursion with pointer addressing
	int32_t offset;
	// register which stores the last address
	// needed for '->', '[ ]', where we have access to heap
	int32_t reg;
	// flag that specifies that we have base address stored in register
	bool in_reg;
	// variant for determing the base address for offset,
	// in case of entity is stack memory, in register case - heap
	StackFrameEntity* entity;
} _addressable_data;

_addressable_data* addressable_data_new()
{
	_addressable_data* data = 
		cnew_s(_addressable_data, data, 1);
	return data;
}

void addressable_data_free(_addressable_data* data)
{
	if (data->in_reg)
		unreserve_register(REGISTERS, data->reg);
	free(data);
}

char* addressible_data_arg(_addressable_data* data)
{
	char* prefix = get_ptr_prefix(data->type);
	if (data->in_reg)
	{
		//todo: i think its not correct
		return frmt("%s ptr [eax+%d]",
			prefix, data->offset);
	}
	else
	{
		return frmt("%s ptr %s[ebp]",
			prefix, data->entity->definition);
	}
}

_addressable_data* gen_addressable_data_for_idnt(
	Idnt* idnt, StackFrame* frame)
{
	_addressable_data* data = addressable_data_new();
	data->entity = get_entity_by_name(idnt->svalue, frame);
	assert(data->entity);
	data->type = data->entity->type;
	data->offset = data->entity->offset;
	return data;
}

_addressable_data* gen_addressable_data(Expr* expr, StackFrame* frame)
{
	/*
		For this kind of expressions we need to
		determine the address, with which we would interact
		As there are too many ways for specifying address (smth in which we can store smth)
		I decided to create separate function, which will return the special
		structure, which describes the specified addressable expression
	*/
	switch (expr->kind)
	{
	case EXPR_IDNT:
		return gen_addressable_data_for_idnt(
			expr->idnt, frame);
	default:
		assert(0);
	}
}

void gen_assign_expr32(BinaryExpr* assign_expr, StackFrame* frame)
{
	_addressable_data* data = NULL;
	char* addr_arg = addressible_data_arg(data =
		gen_addressable_data(assign_expr->lexpr, frame));
	char* eax_reg_arg = get_register_str(EAX);

	// right expression in eax
	gen_expr32(assign_expr->rexpr, frame);

	// reserve register for storing temp result
	int temp_reg = get_unreserved_register(
		REGISTERS, REGSIZE_DWORD);
	char* temp_reg_arg = get_register_str(temp_reg);

	switch (assign_expr->kind)
	{
	case BINARY_ASSIGN:
		PROC_CODE_LINE2(MOV, addr_arg, eax_reg_arg);
		break;
	default:
		// case when its not simple assign
		// first of all cache the address value in temp_reg
		PROC_CODE_LINE2(MOV, temp_reg_arg, addr_arg);
		switch (assign_expr->kind)
		{
		case BINARY_ADD_ASSIGN:
			PROC_CODE_LINE2(ADD, 
				temp_reg_arg, eax_reg_arg);
			break;
		case BINARY_SUB_ASSIGN:
			PROC_CODE_LINE2(SUB, 
				temp_reg_arg, eax_reg_arg);
			break;
		case BINARY_BW_OR_ASSIGN:
			PROC_CODE_LINE2(OR,
				temp_reg_arg, eax_reg_arg);
			break;
		case BINARY_BW_XOR_ASSIGN:
			PROC_CODE_LINE2(XOR,
				temp_reg_arg, eax_reg_arg);
			break;
		case BINARY_BW_AND_ASSIGN:
			PROC_CODE_LINE2(AND,
				temp_reg_arg, eax_reg_arg);
			break;
		case BINARY_BW_NOT_ASSIGN:
			PROC_CODE_LINE1(NOT, temp_reg_arg);
			break;
		case BINARY_LSHIFT_ASSIGN:
			//PROC_CODE_LINE2(SHL,
			//	temp_reg_arg, eax_reg_arg);
			//break;
		case BINARY_RSHIFT_ASSIGN:
			//todo: figure out the shift operators
			assert(0);
			PROC_CODE_LINE2(SHR,
				temp_reg_arg, eax_reg_arg);
			break;

		// result of following operators will be stored in eax:edx
		// but at the end we expecting the temp_reg
		// so mov the result to it (also edx clearing required)
		case BINARY_MUL_ASSIGN:
			reserve_register(REGISTERS, EDX);
			gen_clear_reg(EDX);
			PROC_CODE_LINE1(MUL, temp_reg_arg);
			PROC_CODE_LINE2(MOV, temp_reg_arg, eax_reg_arg);
			unreserve_register(REGISTERS, EDX);
			break;
		case BINARY_DIV_ASSIGN:
			reserve_register(REGISTERS, EDX);
			gen_clear_reg(EDX);
			// put xchg here because we carry about the operators order
			PROC_CODE_LINE2(XCHG, temp_reg_arg, eax_reg_arg);
			PROC_CODE_LINE1(DIV, temp_reg_arg);
			PROC_CODE_LINE2(MOV, temp_reg_arg, eax_reg_arg);
			unreserve_register(REGISTERS, EDX);
			break;
		case BINARY_MOD_ASSIGN:
			reserve_register(REGISTERS, EDX);
			gen_clear_reg(EDX);
			PROC_CODE_LINE2(XCHG, temp_reg_arg, eax_reg_arg);
			PROC_CODE_LINE1(DIV, temp_reg_arg);
			// modulus value stored in edx
			PROC_CODE_LINE2(MOV, temp_reg_arg, 
				get_register_str(EDX));
			unreserve_register(REGISTERS, EDX);
			break;
		}
		// assign calculated value to addressable expr (storage)
		PROC_CODE_LINE2(MOV, addr_arg, temp_reg_arg);
		break;
	}
	// return in eax
	PROC_CODE_LINE2(MOV, eax_reg_arg, addr_arg);

	addressable_data_free(data);
	unreserve_register(REGISTERS, temp_reg);
}

void gen_binary_expr2(BinaryExpr* binary_expr, StackFrame* frame)
{
	#define RESERVE_TEMP_REG  \
		temp_reg = get_unreserved_register(REGISTERS, REGSIZE_DWORD)

	int temp_reg = 0;
	char* arg1 = NULL, arg2 = NULL;

	switch (binary_expr->kind)
	{
	case BINARY_MEMBER_ACCESSOR:
	case BINARY_PTR_MEMBER_ACCESSOR:
	case BINARY_ARR_MEMBER_ACCESSOR:
		return gen_binary_accessor_expr32(binary_expr, frame);

	case BINARY_ASSIGN:
	case BINARY_ADD_ASSIGN:
	case BINARY_SUB_ASSIGN:
	case BINARY_MOD_ASSIGN:
	case BINARY_MUL_ASSIGN:
	case BINARY_DIV_ASSIGN:
	case BINARY_LSHIFT_ASSIGN:
	case BINARY_RSHIFT_ASSIGN:
	case BINARY_BW_OR_ASSIGN:
	case BINARY_BW_NOT_ASSIGN:
	case BINARY_BW_AND_ASSIGN:
	case BINARY_BW_XOR_ASSIGN:
		return gen_assign_expr32(binary_expr, frame);
	}

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
		reserve_register(REGISTERS, EAX);
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
	}
	else
	{
		gen_expr32(binary_expr->lexpr, frame);
		PROC_CODE_LINE1(PUSH, get_register_str(EAX));
		unreserve_register(REGISTERS, EAX);
		gen_expr32(binary_expr->rexpr, frame);
		RESERVE_TEMP_REG;
		PROC_CODE_LINE2(MOV, get_register_str(temp_reg),
			get_register_str(EAX));
		PROC_CODE_LINE1(POP, get_register_str(EAX));
	}

	arg1 = get_register_str(temp_reg);
	arg2 = get_register_str(EAX);
	
	switch (binary_expr->kind)
	{
	//case BINARY_ADD:
	//	ADD32(to, from);
	//	break;
	//case BINARY_SUB:
	//	SUB32(to, from);
	//	break;
	//case BINARY_MOD:
	//	reserve_register(REGISTERS, EDX);
	//	MOD32(to, from);
	//	MOV32(to, get_register_str(EDX));
	//	unreserve_register(REGISTERS, EDX);
	//	break;
	//case BINARY_MULT:
	//	reserve_register(REGISTERS, EDX);
	//	MUL32(to, from);
	//	unreserve_register(REGISTERS, EDX);
	//	break;
	//case BINARY_LSHIFT:
	//	SHL32(to, from);
	//	break;
	//case BINARY_RSHIFT:
	//	SHR32(to, from);
	//	break;
	//case BINARY_BW_OR:
	//	OR32(to, from);
	//	break;
	//case BINARY_BW_AND:
	//	AND32(to, from);
	//	break;
	//case BINARY_BW_XOR:
	//	XOR32(to, from);
	//	break;
	default:
		assert(0);
	}
	unreserve_register(REGISTERS, temp_reg);

#undef GEN_ASSIGN_EXPR
#undef RESERVE_TEMP_REG
}

void gen_func_call32(FuncCall* func_call, StackFrame* frame)
{
	for (int32_t i = sbuffer_len(func_call->args) - 1; i >= 0; i--)
	{
		assert(func_call->args);
		gen_expr32(func_call->args[i], frame);
		unreserve_register(REGISTERS, EAX);
		PROC_CODE_LINE1(PUSH, get_register_str(EAX));
	}
	PROC_CODE_LINE1(CALL, frmt("_%s", func_call->name));
}