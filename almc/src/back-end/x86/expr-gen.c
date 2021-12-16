#include <math.h>

#include "expr-gen.h"

//todo: ADD SIGNED AND UNSIGNED CHECK FOR EXPR-GEN
//todo: struct pointing is still bad (GetLocalTime proofed that)

#define IS_PRIMARY_EXPR(expr)      \
	((expr->kind == EXPR_CONST) || \
	 (expr->kind == EXPR_IDNT))

void gen_const_expr32(Expr* expr)
{
	reserve_register(REGISTERS, EAX);
	PROC_CODE_LINE2(MOV, get_register_str(EAX),
		frmt("%d", evaluate_expr_itype(expr)));
}

void gen_expr32(Expr* expr, StackFrame* frame)
{
	if (is_const_expr(expr, TABLE))
		gen_const_expr32(expr);
	else
	{
		switch (expr->kind)
		{
		case EXPR_IDNT:
		case EXPR_CONST:
			reserve_register(REGISTERS, EAX);
			gen_primary_expr32(expr, EAX, frame);
			break;
		case EXPR_STRING:
			reserve_register(REGISTERS, EAX);
			gen_string32(expr->str, EAX);
			break;
		case EXPR_UNARY_EXPR:
			gen_unary_expr32(expr->unary_expr, frame);
			break;
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

void gen_idnt32(Idnt* idnt, int reg, StackFrame* frame)
{
	//todo: check for enum idnt
	StackFrameEntity* entity = 
		get_entity_by_name(idnt->svalue, frame);
	assert(entity);
	assert(!IS_AGGREGATE_TYPE(idnt->type));

	assert(idnt->type);

	char* reg_arg = get_register_str(
		get_part_of_reg(reg, idnt->type->size * 8));
	char* entity_arg = frmt("%s ptr %s[ebp]", 
		get_ptr_prefix(idnt->type), entity->definition);

	PROC_CODE_LINE2(MOV, reg_arg, entity_arg);
}

void gen_string32(Str* str, int reg)
{
	char** values = NULL;
	sbuffer_add(values, frmt("\"%s\"", str->svalue));
	sbuffer_add(values, frmt("0h"));

	AsmDataLine* line = dataline_new("db", NULL,
		values, DATA_INITIALIZED_STRING);
	char* name = data_add(line, program->data);
	PROC_CODE_LINE2(LEA, get_register_str(reg), name);
}

void gen_primary_expr32(Expr* prim_expr, int reg, StackFrame* frame)
{
	assert(reg >= EAX && reg <= ESI);

	char* reg_arg = get_register_str(reg);
	
	switch (prim_expr->kind)
	{
	case EXPR_CONST:
		switch (prim_expr->cnst->kind)
		{
		case CONST_INT:
			PROC_CODE_LINE2(MOV, reg_arg,
				frmt("%d", prim_expr->cnst->ivalue));
			break;
		case CONST_UINT:
			PROC_CODE_LINE2(MOV, reg_arg,
				frmt("%i", prim_expr->cnst->uvalue));
			break;
		default:
			break;
		}
		break;
	case EXPR_IDNT:
		gen_idnt32(prim_expr->idnt, reg, frame);
		break;
	default:
		assert(0);
	}
}

void gen_unary_sizeof32(UnaryExpr* expr)
{
	PROC_CODE_LINE2(MOV, get_register_str(EAX),
		frmt("%i", expr->cast_type->size));
}

void gen_unary_lengthof32(UnaryExpr* expr)
{
	Type* expr_type = get_expr_type(expr->expr, TABLE);
	PROC_CODE_LINE2(MOV, get_register_str(EAX),
		frmt("%i", expr_type->size));
}

void gen_unary_address32(UnaryExpr* expr, StackFrame* frame)
{
	_addressable_data* data = 
		gen_addressable_data(expr->expr, frame);
	char* data_arg = addressible_data_arg(data);
	PROC_CODE_LINE2(LEA, get_register_str(EAX),
		data_arg);
	addressable_data_free(data);
}

void gen_unary_expr32(UnaryExpr* unary_expr, StackFrame* frame)
{
	char* eax_reg_arg = get_register_str(EAX);

	switch (unary_expr->kind)
	{
	case UNARY_SIZEOF:
		return gen_unary_sizeof32(unary_expr);
	case UNARY_LENGTHOF:
		return gen_unary_lengthof32(unary_expr);
	case UNARY_CAST:
		return gen_expr32(unary_expr->expr, frame);
	case UNARY_ADDRESS:
		return gen_unary_address32(unary_expr, frame);
	}

	if (!IS_PRIMARY_EXPR(unary_expr->expr))
		gen_expr32(unary_expr->expr, frame);
	else
		reserve_register(REGISTERS, EAX),
			gen_primary_expr32(unary_expr->expr, EAX, frame);

	switch (unary_expr->kind)
	{
	case UNARY_PLUS:
		break;
	case UNARY_MINUS:
		PROC_CODE_LINE1(NEG,
			eax_reg_arg);
		break;
	case UNARY_BW_NOT:
		PROC_CODE_LINE1(NOT,
			eax_reg_arg);
		break;
	case UNARY_DEREFERENCE:
		assert(0);
		PROC_CODE_LINE2(MOV, eax_reg_arg,
			frmt("dword ptr [eax]"));
		break;
	default:
		assert(0);
	}
}

_addressable_data* gen_accessor_data(BinaryExpr* expr, StackFrame* frame)
{
	_addressable_data* data = NULL;
	if (expr->lexpr->kind != EXPR_IDNT)
		data = gen_accessor_data(expr->lexpr->binary_expr, frame);
	else
	{
		data = cnew_s(_addressable_data, data, 1);
		data->type = expr->lexpr->idnt->type;
		data->in_reg = false;
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
		if (data->in_reg)
			arg2 = frmt("%s ptr [eax+%d]",
				get_ptr_prefix(data->type), data->offset);
		else
		{
			data->in_reg = true;
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
		if (data->in_reg)
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
		if (data->in_reg)
		{
			arg2 = frmt("%s ptr [%s+eax*%d]", get_ptr_prefix(data->type->base),
				get_register_str(temp_reg), data->type->base->size);
			unreserve_register(REGISTERS, temp_reg);
		}
		else
		{
			data->in_reg = true;
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
	_addressable_data* data = gen_accessor_data(expr, frame);

	assert(data->type);
	assert(!IS_AGGREGATE_TYPE(data->type));

	char* arg1 = get_register_str(
		get_part_of_reg(EAX, data->type->size * 8));
	char* arg2 = NULL;
	// calculating the address that will be relative to bottom of the allocate space
	// this magic -1 is bruted for perfect fitting to the bounds of allocated space
	//data->offset = abs(data->offset - 
	//	(int32_t)data->entity->type->size) - 1;
	if (!data->in_reg)
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

_addressable_data* gen_addressable_data_for_idnt(
	Idnt* idnt, StackFrame* frame)
{
	_addressable_data* data = addressable_data_new();
	data->entity = get_entity_by_name(idnt->svalue, frame);
	assert(data->entity);
	data->kind = ADDRESSABLE_ENTITY;
	data->type = data->entity->type;
	return data;
}

_addressable_data* gen_addressable_data_for_accessor(
	BinaryExpr* expr, StackFrame* frame)
{
	char* arg1 = NULL;
	char* arg2 = NULL;
	_addressable_data* data = NULL;

	int addr_reg = 0;
	int temp_reg = 0;

	if (expr->lexpr->kind != EXPR_IDNT)
		data = gen_addressable_data_for_accessor(
			expr->lexpr->binary_expr, frame);
	else
		data = gen_addressable_data_for_idnt(
			expr->lexpr->idnt, frame);

	switch (expr->kind)
	{
	case BINARY_MEMBER_ACCESSOR:
		data->kind = expr->kind == BINARY_MEMBER_ACCESSOR ? 
			ADDRESSABLE_ACCESSOR : ADDRESSABLE_PTR_ACCESSOR;
		assert(IS_STRUCT_TYPE(data->type));
		for (size_t i = 0; i < sbuffer_len(data->type->members); i++)
			if (strcmp(data->type->members[i]->name, expr->rexpr->idnt->svalue) == 0)
				return data->offset += data->type->members[i]->offset,
					data->type = data->type->members[i]->type, data;
		assert(0);
		break;
	}
	return data;
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

void gen_relative_expr32(BinaryExpr* relative_expr, StackFrame* frame)
{
	assert(0);
	gen_expr32(relative_expr->lexpr, frame);

	int temp_reg = get_unreserved_register(REGISTERS,
		REGSIZE_DWORD);
	char* temp_reg_arg = get_register_str(temp_reg);
	
	switch (relative_expr->kind)
	{
	case BINARY_LG_OR:
	case BINARY_LG_AND:
	case BINARY_LG_EQ:
	case BINARY_LG_NEQ:

	case BINARY_LESS_THAN:
	case BINARY_GREATER_THAN:
	case BINARY_LESS_EQ_THAN:
	case BINARY_GREATER_EQ_THAN:
		break;
	default:
		assert(0);
	}
}

void gen_binary_expr32(BinaryExpr* binary_expr, StackFrame* frame)
{
	#define RESERVE_TEMP_REG  \
		temp_reg = get_unreserved_register(REGISTERS, REGSIZE_DWORD)

	int temp_reg = 0;
	char* eax_reg_arg = NULL;
	char* temp_reg_arg = NULL;

	switch (binary_expr->kind)
	{
	case BINARY_MEMBER_ACCESSOR:
	case BINARY_PTR_MEMBER_ACCESSOR:
	case BINARY_ARR_MEMBER_ACCESSOR:
		return gen_binary_accessor_expr32(binary_expr, frame);

	case BINARY_LG_OR:
	case BINARY_LG_AND:
	case BINARY_LG_EQ:
	case BINARY_LG_NEQ:

	case BINARY_LESS_THAN:
	case BINARY_GREATER_THAN:
	case BINARY_LESS_EQ_THAN:
	case BINARY_GREATER_EQ_THAN:
		return gen_relative_expr32(binary_expr, frame);

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

	eax_reg_arg = get_register_str(EAX);
	temp_reg_arg = get_register_str(temp_reg);
	
	switch (binary_expr->kind)
	{
	case BINARY_ADD:
		PROC_CODE_LINE2(ADD, eax_reg_arg,
			temp_reg_arg);
		break;
	case BINARY_SUB:
		PROC_CODE_LINE2(SUB, eax_reg_arg,
			temp_reg_arg);
		break;
	case BINARY_LSHIFT:
	case BINARY_RSHIFT:
		assert(0);
		break;
	case BINARY_BW_OR:
		PROC_CODE_LINE2(OR, eax_reg_arg,
			temp_reg_arg);
		break;
	case BINARY_BW_AND:
		PROC_CODE_LINE2(AND, eax_reg_arg,
			temp_reg_arg);
		break;
	case BINARY_BW_XOR:
		PROC_CODE_LINE2(XOR, eax_reg_arg,
			temp_reg_arg);
		break;
	case BINARY_MOD:
		reserve_register(REGISTERS, EDX);
		gen_clear_reg(EDX);
		PROC_CODE_LINE1(DIV, temp_reg_arg);
		PROC_CODE_LINE2(MOV, eax_reg_arg, 
			get_register_str(EDX));
		unreserve_register(REGISTERS, EDX);
		break;
	case BINARY_DIV:
		reserve_register(REGISTERS, EDX);
		gen_clear_reg(EDX);
		PROC_CODE_LINE1(DIV, temp_reg_arg);
		unreserve_register(REGISTERS, EDX);
		break;
	case BINARY_MULT:
		reserve_register(REGISTERS, EDX);
		gen_clear_reg(EDX);
		PROC_CODE_LINE1(MUL, temp_reg_arg);
		unreserve_register(REGISTERS, EDX);
		break;
	default:
		assert(0);
	}
	unreserve_register(REGISTERS, temp_reg);

#undef RESERVE_TEMP_REG
}

void gen_callee_stack_clearing(FuncDecl* func_decl)
{
	uint32_t size = 0;
	switch (func_decl->conv->kind)
	{
	case CALL_CONV_CDECL:
		/*
			if cdecl:
				...
				ret
		*/
		PROC_CODE_LINE0(RET);
		break;
	case CALL_CONV_STDCALL:
		/*
			if stdcall:
				...
				ret  xxxx <- overall size of all arguments
		*/
		for (size_t i = 0; i < sbuffer_len(func_decl->params); i++)
			size += func_decl->params[i]->type->size;
		PROC_CODE_LINE1(RET, frmt("%d", size));
		break;
	}
}

void gen_caller_stack_clearing(FuncCall* func_call)
{
	uint32_t size = 0;
	switch (func_call->conv->kind)
	{
	case CALL_CONV_CDECL:
		/*
			if cdecl:
				call xxxx
				add  esp, yyyy
		*/
		// caching the size of each passed argument 
		// to clear this space
		for (size_t i = 0; i < sbuffer_len(func_call->args); i++)
			size += retrieve_expr_type(func_call->args[i])->size;
		PROC_CODE_LINE2(ADD, get_register_str(ESP),
			frmt("%d", size));
		break;
	case CALL_CONV_STDCALL:
		/*
			if stdcall:
				call xxxx
				; nothing do here because calle will free the stack
		*/
		break;
	}
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
	// if we met here the external function there will be no need to add underscore
	PROC_CODE_LINE1(CALL, frmt(func_call->spec->is_external ? 
		"%s" : "_%s", func_call->name));
	gen_caller_stack_clearing(func_call);
}

char* addressible_data_arg(_addressable_data* data)
{
	char* prefix = get_ptr_prefix(data->type);

	switch (data->kind)
	{
	case ADDRESSABLE_ENTITY:
		return frmt("%s ptr %s[ebp]",
			prefix, data->entity->definition);
	case ADDRESSABLE_ACCESSOR:
		if (data->in_reg)
			return frmt("%s ptr [%s+%d]", prefix,
				get_register_str(data->reg), data->offset);
		else
			return frmt("%s ptr %s[ebp+%d]", prefix,
				data->entity->definition, data->offset);
	default:
		assert(0);
	}
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
	case EXPR_BINARY_EXPR:
		switch (expr->binary_expr->kind)
		{
		case BINARY_MEMBER_ACCESSOR:
		case BINARY_PTR_MEMBER_ACCESSOR:
		case BINARY_ARR_MEMBER_ACCESSOR:
			return gen_addressable_data_for_accessor(
				expr->binary_expr, frame);
		}
		assert(0);
		break;
	default:
		assert(0);
	}
}

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