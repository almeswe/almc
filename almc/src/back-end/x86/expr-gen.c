#include "expr-gen.h"

//todo: ADD SIGNED AND UNSIGNED CHECK FOR EXPR-GEN
//todo: add support for labels with the same name in different scopes

//todo: do global clean-up for back-end
//todo: finish visit_array_accessor function

#define IS_PRIMARY_EXPR(expr)      \
	((expr->kind == EXPR_CONST) || \
	 (expr->kind == EXPR_IDNT))

void gen_const_expr32(Expr* expr)
{
	reserve_register(REGISTERS, EAX);
	PROC_CODE_LINE2(MOV, get_register_str(EAX), 
		frmt("%d", evaluate_expr_itype(expr)));
}

void gen_mov_reg_to32(int reg, char* data, Type* datatype)
{
	int reg_part = get_part_of_reg(reg, 
		datatype->size * 8);
	char* reg_arg = get_register_str(reg_part);
	PROC_CODE_LINE2(MOV, data, get_register_str(reg_part));
}

void gen_mov_to_reg32(int reg, char* data, Type* datatype)
{
	char* reg_arg = get_register_str(reg);
	if (datatype->size >= 4)
		PROC_CODE_LINE2(MOV, reg_arg, data);
	else
	{
		if (is_signed_type(datatype))
			PROC_CODE_LINE2(MOVSX, reg_arg, data);
		else if (is_unsigned_type(datatype))
			PROC_CODE_LINE2(MOVZX, reg_arg, data);
	}
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
			reserve_register(REGISTERS, EAX),
				gen_primary_expr32(expr, EAX, frame);
			break;
		case EXPR_STRING:
			reserve_register(REGISTERS, EAX),
				gen_string32(expr->str, EAX);
			break;
		case EXPR_UNARY_EXPR:
			reserve_register(REGISTERS, EAX), 
				gen_unary_expr32(expr->unary_expr, frame);
			break;
		case EXPR_BINARY_EXPR:
			gen_binary_expr32(expr->binary_expr, frame);
			break;
		case EXPR_TERNARY_EXPR:
			gen_ternary_expr(expr->ternary_expr, frame);
			break;
		case EXPR_FUNC_CALL:
			gen_func_call32(expr->func_call, frame);
			break;
		default:
			report_error("Unknown expression kind met."
			 " in gen_expr32", NULL);
		}
	}
}

void gen_idnt32(Idnt* idnt, int reg, StackFrame* frame)
{
	if (idnt->is_enum_member)
		gen_expr32(idnt->enum_member_value, frame);
	else
	{
		StackFrameEntity* entity =
			get_entity_by_name(idnt->svalue, frame);
		assert(entity);
		assert(idnt->type);
		char* reg_arg = get_register_str(
			get_part_of_reg(reg, idnt->type->size * 8));
		char* entity_arg = frmt("%s ptr %s[ebp]",
			get_ptr_prefix(idnt->type), entity->definition);

		PROC_CODE_LINE2(IS_AGGREGATE_TYPE(idnt->type) ? LEA : MOV, 
			reg_arg, entity_arg);
	}
}

void gen_string32(Str* str, int reg)
{
	//todo: refactor gen_string32
	char* value = frmt("\"");
	char* prev = NULL;
	char** values = NULL;
	bool value_is_not_empty = false;
	size_t len = strlen(str->svalue);

	for (size_t i = 0; i < len; i++)
	{
		if (!isescape(str->svalue[i]))
		{
			value = frmt("%s%c", prev = value, str->svalue[i]), free(prev);
			value_is_not_empty = true;
		}
		else
		{
			value = frmt("%s\"", prev = value), free(prev);
			if (value_is_not_empty)
				sbuffer_add(values, value);
			sbuffer_add(values, frmt("%02xh", (int)str->svalue[i]));
			value = frmt("\"");
			value_is_not_empty = false;
		}
	}
	if ((prev = value) && value_is_not_empty)
		sbuffer_add(values, value = frmt("%s\"", value)), free(prev);
	sbuffer_add(values, frmt("00h"));

	AsmDataLine* line = dataline_new("db", NULL,
		values, DATA_INITIALIZED_STRING);
	char* name = data_add(line, program->data);
	PROC_CODE_LINE2(LEA, get_register_str(reg), name);
}

void gen_primary_expr32(Expr* prim_expr, int reg, StackFrame* frame)
{
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
		case CONST_CHAR:
			PROC_CODE_LINE2(MOVZX, reg_arg,
				frmt("%d", (char)prim_expr->cnst->ivalue));
			break;
		default:
			report_error("Unknown const kind met."
				" in gen_primary_expr32", NULL);
			break;
		}
		break;
	case EXPR_IDNT:
		gen_idnt32(prim_expr->idnt, reg, frame);
		break;
	default:
		report_error("Unknown primary expression kind met."
		 " in gen_primary_expr32", NULL);
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
	char* data_arg = addressable_data_arg(data);
	PROC_CODE_LINE2(LEA, get_register_str(EAX),
		data_arg);
	addressable_data_free(data);
}

void gen_unary_dereference32(UnaryExpr* expr, StackFrame* frame)
{
	gen_expr32(expr->expr, frame);
	PROC_CODE_LINE2(MOV, get_register_str(EAX), frmt("[eax]"));
}

void gen_unary_lg_not32(UnaryExpr* expr)
{
	char* eax_reg_arg = get_register_str(EAX);
	NEW_LABEL(label_true);
	NEW_LABEL(label_end);

	PROC_CODE_LINE2(CMP, eax_reg_arg, 
		LOGICAL_FALSE_ARG);
	PROC_CODE_LINE1(JE, label_true);
	PROC_CODE_LINE2(MOV, eax_reg_arg, 
		LOGICAL_FALSE_ARG);
	PROC_CODE_LINE1(JMP, label_end);
	PROC_CODE_LINE1(_LABEL, label_true);
	PROC_CODE_LINE2(MOV, eax_reg_arg, 
		LOGICAL_TRUE_ARG);
	PROC_CODE_LINE1(_LABEL, label_end);
}

void gen_unary_expr32(UnaryExpr* unary_expr, StackFrame* frame)
{
	char* eax_reg_arg = get_register_str(EAX);
	switch (unary_expr->kind)
	{
	case UNARY_SIZEOF:
		gen_unary_sizeof32(unary_expr);
		break;
	case UNARY_LENGTHOF:
		gen_unary_lengthof32(unary_expr);
		break;
	case UNARY_CAST:
		gen_expr32(unary_expr->expr, frame);
		break;
	case UNARY_ADDRESS:
		gen_unary_address32(unary_expr, frame);
		break;
	case UNARY_DEREFERENCE:
		gen_unary_dereference32(unary_expr, frame);
		break;
	default:
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
		case UNARY_LG_NOT:
			gen_unary_lg_not32(unary_expr);
			break;
		default:
			report_error("Unknown unary expression kind met."
				" in gen_unary_expr32", NULL);
		}
	}
}

void gen_binary_accessor_expr32(BinaryExpr* expr, StackFrame* frame)
{
	// wraps the binary expression to expression, needed for
	// gen_addressable_data interface
	Expr* wrapper = expr_new(EXPR_BINARY_EXPR, expr);

	_addressable_data* data = NULL;
	char* addr_arg = addressable_data_arg(data =
		gen_addressable_data(wrapper, frame));
	reserve_register(REGISTERS, EAX);
	gen_mov_to_reg32(EAX, addr_arg, data->type);
	addressable_data_free(data), free(wrapper);
}

void gen_reg_clear(int reg)
{
	PROC_CODE_LINE2(XOR, get_register_str(reg),
		get_register_str(reg));
}

void gen_reg_arg_clear(char* reg_arg)
{
	PROC_CODE_LINE2(XOR, reg_arg, reg_arg);
}

void gen_binary_comma_expr32(BinaryExpr* expr, StackFrame* frame)
{
	gen_expr32(expr->lexpr, frame);
	unreserve_register(REGISTERS, EAX);
	gen_expr32(expr->rexpr, frame);
}

void gen_binary_assign_expr32(BinaryExpr* assign_expr, StackFrame* frame)
{
	_addressable_data* data = NULL;
	char* addr_arg = addressable_data_arg(data =
		gen_addressable_data(assign_expr->lexpr, frame));
	char* eax_reg_arg = get_register_str(EAX);
	// if we have address in register and it is EAX, 
	// we need to save it because we'll need it for storing value of left side
	if (data->in_reg && data->reg == EAX)
		PROC_CODE_LINE1(PUSH, get_register_str(EAX));
	// right expression in eax
	gen_expr32(assign_expr->rexpr, frame);

	// reserve register for storing temp result
	int temp_reg = get_unreserved_register(
		REGISTERS, REGSIZE_DWORD);
	char* temp_reg_arg = get_register_str(temp_reg);

	switch (assign_expr->kind)
	{
	case BINARY_ASSIGN:
		if (data->in_reg && data->reg == EAX)
		{
			PROC_CODE_LINE2(MOV, temp_reg_arg, eax_reg_arg);
			PROC_CODE_LINE1(POP, get_register_str(EAX));
			gen_mov_reg_to32(temp_reg, addr_arg, 
				retrieve_expr_type(assign_expr->lexpr));
		}
		else
			gen_mov_reg_to32(EAX, addr_arg,
				retrieve_expr_type(assign_expr->lexpr));
		break;
	default:
		// case when its not simple assign
		// first of all cache the address value in temp_reg
		gen_mov_to_reg32(temp_reg, addr_arg,
			retrieve_expr_type(assign_expr->lexpr));
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
		case BINARY_LSHIFT_ASSIGN:
		case BINARY_RSHIFT_ASSIGN:
			PROC_CODE_LINE2(XCHG, temp_reg_arg, eax_reg_arg);
			if (temp_reg != ECX)
			{
				reserve_register(REGISTERS, ECX);
				PROC_CODE_LINE2(MOV, get_register_str(ECX), 
					temp_reg_arg);
				unreserve_register(REGISTERS, ECX);
			}
			PROC_CODE_LINE2(assign_expr->kind == BINARY_LSHIFT_ASSIGN ?
				SHL : SHR, eax_reg_arg, get_register_str(CL));
			PROC_CODE_LINE2(MOV, temp_reg_arg, eax_reg_arg);
			break;

		// result of following operators will be stored in eax:edx
		// but at the end we expecting the temp_reg
		// so mov the result to it (also edx clearing required)
		case BINARY_MUL_ASSIGN:
			reserve_register(REGISTERS, EDX);
			gen_reg_clear(EDX);
			PROC_CODE_LINE1(MUL, temp_reg_arg);
			PROC_CODE_LINE2(MOV, temp_reg_arg, eax_reg_arg);
			unreserve_register(REGISTERS, EDX);
			break;
		case BINARY_DIV_ASSIGN:
			reserve_register(REGISTERS, EDX);
			gen_reg_clear(EDX);
			// put xchg here because we carry about the operators order
			PROC_CODE_LINE2(XCHG, temp_reg_arg, eax_reg_arg);
			PROC_CODE_LINE1(DIV, temp_reg_arg);
			PROC_CODE_LINE2(MOV, temp_reg_arg, eax_reg_arg);
			unreserve_register(REGISTERS, EDX);
			break;
		case BINARY_MOD_ASSIGN:
			reserve_register(REGISTERS, EDX);
			gen_reg_clear(EDX);
			PROC_CODE_LINE2(XCHG, temp_reg_arg, eax_reg_arg);
			PROC_CODE_LINE1(DIV, temp_reg_arg);
			// modulus value stored in edx
			PROC_CODE_LINE2(MOV, temp_reg_arg, 
				get_register_str(EDX));
			unreserve_register(REGISTERS, EDX);
			break;
		default:
			report_error("Unknown binary assign expression kind met."
			" in gen_binary_assign_expr32", NULL);
		}
		if (data->in_reg && data->reg == EAX)
			PROC_CODE_LINE1(POP, get_register_str(EAX));
		// assign calculated value to addressable expr (storage)
		gen_mov_reg_to32(temp_reg, addr_arg,
			retrieve_expr_type(assign_expr->lexpr));
		break;
	}
	// return in eax, or in one of its lower parts
	gen_mov_to_reg32(EAX, addr_arg,
		retrieve_expr_type(assign_expr->lexpr));

	addressable_data_free(data);
	unreserve_register(REGISTERS, temp_reg);
}

void gen_binary_relative_expr32(BinaryExpr* relative_expr, StackFrame* frame)
{
#define SIGN_BASED(sign_instr, unsign_instr, type) \
	(is_signed_type(type) ? sign_instr, unsign_instr)

	gen_expr32(relative_expr->lexpr, frame);

	char* eax_reg_arg = get_register_str(EAX);
	PROC_CODE_LINE1(PUSH, eax_reg_arg);
	
	gen_expr32(relative_expr->rexpr, frame);
	int temp_reg = get_unreserved_register(
		REGISTERS, REGSIZE_DWORD);
	char* temp_reg_arg = get_register_str(temp_reg);
	PROC_CODE_LINE2(MOV, temp_reg_arg, eax_reg_arg);
	PROC_CODE_LINE1(POP, eax_reg_arg);

	char* label_supply = program_new_label(program);
	char* label_final = program_new_label(program);

	// exceptional case for logical and:
	if (relative_expr->kind == BINARY_LG_AND || 
		relative_expr->kind == BINARY_LG_OR)
	{
		switch (relative_expr->kind)
		{
		case BINARY_LG_OR:
			PROC_CODE_LINE2(CMP, eax_reg_arg, 
				LOGICAL_FALSE_ARG);
			PROC_CODE_LINE1(JNE, label_supply);
			PROC_CODE_LINE2(CMP, temp_reg_arg, 
				LOGICAL_FALSE_ARG);
			PROC_CODE_LINE1(JNE, label_supply);

			PROC_CODE_LINE2(MOV, eax_reg_arg, 
				LOGICAL_FALSE_ARG);
			PROC_CODE_LINE1(JMP, label_final);
			PROC_CODE_LINE1(_LABEL, label_supply);
			PROC_CODE_LINE2(MOV, eax_reg_arg, 
				LOGICAL_TRUE_ARG);
			PROC_CODE_LINE1(_LABEL, label_final);
			break;
		case BINARY_LG_AND:
			PROC_CODE_LINE2(CMP, eax_reg_arg, 
				LOGICAL_FALSE_ARG);
			PROC_CODE_LINE1(JE, label_supply);
			PROC_CODE_LINE2(CMP, temp_reg_arg, 
				LOGICAL_FALSE_ARG);
			PROC_CODE_LINE1(JE, label_supply);

			PROC_CODE_LINE2(MOV, eax_reg_arg, 
				LOGICAL_TRUE_ARG);
			PROC_CODE_LINE1(JMP, label_final);
			PROC_CODE_LINE1(_LABEL, label_supply);
			PROC_CODE_LINE2(MOV, eax_reg_arg, 
				LOGICAL_FALSE_ARG);
			PROC_CODE_LINE1(_LABEL, label_final);
			break;
		}
	}
	else
	{
		PROC_CODE_LINE2(CMP, eax_reg_arg,
			temp_reg_arg);
		switch (relative_expr->kind)
		{
		case BINARY_LESS_THAN:
			PROC_CODE_LINE1(JL, label_supply);
			break;
		case BINARY_GREATER_THAN:
			PROC_CODE_LINE1(JG, label_supply);
			break;
		case BINARY_LESS_EQ_THAN:
			PROC_CODE_LINE1(JLE, label_supply);
			break;
		case BINARY_GREATER_EQ_THAN:
			PROC_CODE_LINE1(JGE, label_supply);
			break;
		case BINARY_LG_EQ:
			PROC_CODE_LINE1(JE, label_supply);
			break;
		case BINARY_LG_NEQ:
			PROC_CODE_LINE1(JNE, label_supply);
			break;
		default:
			assert(0);
		}
		PROC_CODE_LINE2(MOV, eax_reg_arg, 
			LOGICAL_FALSE_ARG);
		PROC_CODE_LINE1(JMP, label_final);
		PROC_CODE_LINE1(_LABEL, label_supply);
		PROC_CODE_LINE2(MOV, eax_reg_arg, 
			LOGICAL_TRUE_ARG);
		PROC_CODE_LINE1(_LABEL, label_final);
	}
	unreserve_register(REGISTERS, temp_reg);
}

void gen_binary_expr32(BinaryExpr* binary_expr, StackFrame* frame)
{
	#define RESERVE_TEMP_REG  \
		temp_reg = get_unreserved_register(\
			REGISTERS, REGSIZE_DWORD)

	int temp_reg = 0;
	char* eax_reg_arg = NULL;
	char* temp_reg_arg = NULL;

	switch (binary_expr->kind)
	{
	case BINARY_MEMBER_ACCESSOR:
	case BINARY_PTR_MEMBER_ACCESSOR:
	case BINARY_ARR_MEMBER_ACCESSOR:
		gen_binary_accessor_expr32(binary_expr, frame);
		break;

	case BINARY_LG_OR:
	case BINARY_LG_AND:
	case BINARY_LG_EQ:
	case BINARY_LG_NEQ:
	case BINARY_LESS_THAN:
	case BINARY_GREATER_THAN:
	case BINARY_LESS_EQ_THAN:
	case BINARY_GREATER_EQ_THAN:
		gen_binary_relative_expr32(binary_expr, frame);
		break;

	case BINARY_ASSIGN:
	case BINARY_ADD_ASSIGN:
	case BINARY_SUB_ASSIGN:
	case BINARY_MOD_ASSIGN:
	case BINARY_MUL_ASSIGN:
	case BINARY_DIV_ASSIGN:
	case BINARY_LSHIFT_ASSIGN:
	case BINARY_RSHIFT_ASSIGN:
	case BINARY_BW_OR_ASSIGN:
	case BINARY_BW_AND_ASSIGN:
	case BINARY_BW_XOR_ASSIGN:
		gen_binary_assign_expr32(binary_expr, frame);
		break;
	
	case BINARY_COMMA:
		gen_binary_comma_expr32(binary_expr, frame);
		break;

	default:
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
			// the second operator need to be in cl register for shift operators
		case BINARY_LSHIFT:
		case BINARY_RSHIFT:
			if (temp_reg != ECX)
			{
				reserve_register(REGISTERS, ECX);
				PROC_CODE_LINE2(MOV,
					get_register_str(ECX), temp_reg_arg);
				unreserve_register(REGISTERS, ECX);
			}
			PROC_CODE_LINE2(binary_expr->kind == BINARY_LSHIFT ? SHL : SHR,
				eax_reg_arg, get_register_str(CL));
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
			gen_reg_clear(EDX);
			PROC_CODE_LINE1(DIV, temp_reg_arg);
			PROC_CODE_LINE2(MOV, eax_reg_arg,
				get_register_str(EDX));
			unreserve_register(REGISTERS, EDX);
			break;
		case BINARY_DIV:
			reserve_register(REGISTERS, EDX);
			gen_reg_clear(EDX);
			PROC_CODE_LINE1(DIV, temp_reg_arg);
			unreserve_register(REGISTERS, EDX);
			break;
		case BINARY_MULT:
			reserve_register(REGISTERS, EDX);
			gen_reg_clear(EDX);
			PROC_CODE_LINE1(MUL, temp_reg_arg);
			unreserve_register(REGISTERS, EDX);
			break;
		default:
			report_error("Unknown binary expression kind met."
				" in gen_binary_expr32()", NULL);
		}
		unreserve_register(REGISTERS, temp_reg);
	}

#undef RESERVE_TEMP_REG
}

void gen_ternary_expr(TernaryExpr* ternary_expr, StackFrame* frame)
{
	// condition:
	gen_expr32(ternary_expr->cond, frame);
	unreserve_register(REGISTERS, EAX);
	NEW_LABEL(label_false_branch);
	NEW_LABEL(label_end);

	char* eax_reg_arg = get_register_str(EAX);
	PROC_CODE_LINE2(CMP, eax_reg_arg, LOGICAL_FALSE_ARG);
	PROC_CODE_LINE1(JE, label_false_branch);
	gen_expr32(ternary_expr->lexpr, frame);
	unreserve_register(REGISTERS, EAX);
	PROC_CODE_LINE1(JMP, label_end);
	PROC_CODE_LINE1(_LABEL, label_false_branch);
	gen_expr32(ternary_expr->rexpr, frame);
	unreserve_register(REGISTERS, EAX);
	PROC_CODE_LINE1(_LABEL, label_end);
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
		{
			Type* type = func_decl->params[i]->type;
			assert(type);
			size += IS_AGGREGATE_TYPE(type) ? MACHINE_WORD : type->size;
		}
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
		{
			Type* type = retrieve_expr_type(func_call->args[i]);
			// array struct or union types passed by reference, not by value
			// like pointer but which points to stack.
			assert(type);
			size += IS_AGGREGATE_TYPE(type) ? MACHINE_WORD : type->size;
		}
		if (size)
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
	int* registers = cache_general_purpose_registers();
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
	restore_general_purpose_registers(registers);
}

int* cache_general_purpose_registers()
{
	int* regs = NULL;
	for (size_t reg = EBX; reg <= EDX; reg += 3)
		if (REGISTERS->reg_table[reg] == REGISTER_RESERVED)
			sbuffer_add(regs, reg), PROC_CODE_LINE1(PUSH, 
				get_register_str(reg));
	return regs;
}

void restore_general_purpose_registers(int* regs)
{
	for (size_t reg = 0; reg < sbuffer_len(regs); reg++)
		PROC_CODE_LINE1(POP, get_register_str(regs[reg]));
	sbuffer_free(regs);
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

char* addressable_data_arg(_addressable_data* data)
{
	/*
		Function that converts addressable data to
		argument for using in PROC_CODE_LINEx
	*/
	char* prefix = get_ptr_prefix(data->type);
	switch (data->kind)
	{
	case ADDRESSABLE_ENTITY:
		if (data->in_reg)
			return frmt("%s", data->reg);
		else
			return frmt("%s ptr %s[ebp]",
				prefix, data->entity->definition);
	case ADDRESSABLE_ACCESSOR:
		if (data->in_reg)
			return frmt("%s ptr [%s+%d]", prefix,
				get_register_str(data->reg), data->offset);
		else
			return frmt("%s ptr %s[ebp+%d]", prefix,
				data->entity->definition, data->offset);
	case ADDRESSABLE_ARR_ACCESSOR:
	case ADDRESSABLE_PTR_ACCESSOR:
		return frmt("%s ptr [%s]", prefix,
			get_register_str(data->reg));
	case ADDRESSABLE_DEREFERENCE:
		return frmt("dword ptr [%s]", get_register_str(data->reg));
	default:
		report_error("Unsupported addressable data kind met."
			" in addressable_data_arg.", NULL);
	}
	return NULL;
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
	case EXPR_UNARY_EXPR:
		switch (expr->unary_expr->kind)
		{
		case UNARY_DEREFERENCE:
			return gen_addressable_data_for_dereference(
				expr->unary_expr, frame);
		default:
			assert(0);
		}
		break;
	case EXPR_BINARY_EXPR:
		switch (expr->binary_expr->kind)
		{
		case BINARY_MEMBER_ACCESSOR:
		case BINARY_PTR_MEMBER_ACCESSOR:
		case BINARY_ARR_MEMBER_ACCESSOR:
			return gen_addressable_data_for_accessor(
				expr->binary_expr, frame);
		default:
			assert(0);
		}
		break;
	default:
		assert(0);
	}
	return NULL;
}

_addressable_data* gen_addressable_data_for_idnt(
	Idnt* idnt, StackFrame* frame)
{
	_addressable_data* data = addressable_data_new();
	data->capacity = 1;
	data->entity = get_entity_by_name(idnt->svalue, frame);
	assert(data->entity);
	data->kind = ADDRESSABLE_ENTITY;
	data->type = data->origin = data->entity->type;
	data->dimension = get_array_dimensions(data->type);

	switch (data->entity->kind)
	{
	// if we met the aggregate type as passed argument
	// it means that the reference is stored in this ceil, not actual
	// struct, union or array
	// so need to put this address in to the register
	case STACK_FRAME_ENTITY_ARGUMENT:
		if (IS_AGGREGATE_TYPE(data->entity->type))
		{
			data->in_reg = true;
			data->reg = get_unreserved_register(
				REGISTERS, REGSIZE_DWORD);
			PROC_CODE_LINE2(MOV, get_register_str(data->reg),
				frmt("dword ptr %s[ebp]", data->entity->definition));
		}
		break;
	default:
		data->in_reg = false;
		break;
	}
	return data;
}

_addressable_data* gen_addressable_data_for_dereference(
	UnaryExpr* expr, StackFrame* frame)
{
	_addressable_data* data = addressable_data_new();
	gen_expr32(expr->expr, frame);
	data->reg = EAX;
	data->in_reg = true;
	data->type = expr->type;
	data->kind = ADDRESSABLE_DEREFERENCE;
	return data;
}

_addressable_data* gen_addressable_data_for_accessor(
	BinaryExpr* expr, StackFrame* frame)
{
	_addressable_data* data = gen_addressable_data(
		expr->lexpr, frame);

	switch (expr->kind)
	{
	case BINARY_ARR_MEMBER_ACCESSOR:
		return gen_addressable_data_for_array_accessor(
			data, expr, frame);
	case BINARY_MEMBER_ACCESSOR:
		return gen_addressable_data_for_struct_accessor(
			data, expr, frame);
	case BINARY_PTR_MEMBER_ACCESSOR:
		return gen_addressable_data_for_struct_ptr_accessor(
			data, expr, frame);
	default:
		report_error("Passed expression is not the accessor actually."
			" in gen...for_accessor", NULL);
	}
	return data;
}

_addressable_data* gen_addressable_data_for_array_accessor(
	_addressable_data* data, BinaryExpr* expr, StackFrame* frame)
{
	gen_expr32(expr->rexpr, frame);

	data->kind = ADDRESSABLE_ARR_ACCESSOR;
	Type* ltype = retrieve_expr_type(expr->lexpr);
	if (!ltype)
		report_error("Cannot retrieve ltype"
			" in gen_addressable_data_for_array_accessor()", NULL);
	uint32_t capacity = IS_ARRAY_TYPE(ltype) ? (uint32_t)evaluate_expr_itype(
		get_array_dimension(ltype, data->dimension)) : 1;
	uint32_t typesize = IS_POINTER_TYPE(ltype) ? 
		ltype->base->size : get_array_base_type(ltype)->size;

	if (!data->in_reg)
		data->reg = get_unreserved_register(
			REGISTERS, REGSIZE_DWORD);

	int scalar_reg = get_unreserved_register(
		REGISTERS, REGSIZE_DWORD);

	char* addr_arg = NULL;
	char* eax_reg_arg = get_register_str(EAX);
	char* temp_reg_arg = get_register_str(data->reg);
	char* scalar_reg_arg = get_register_str(scalar_reg);

	if (data->in_reg)
	{
		// the formula of [reg+eax*scalar_reg] is not used here 
		// because it requires more logic for check tje scalar_reg (it accepts 2,4 or 8)
		// in this case just reserve new reg on which we will multiply value stored in eax, then use it 
		// for array member access
		PROC_CODE_LINE2(MOV, scalar_reg_arg,
			frmt("%d", typesize * data->capacity));
		PROC_CODE_LINE1(MUL, scalar_reg_arg);
		addr_arg = frmt("dword ptr [%s+eax]",
			temp_reg_arg);
	}
	else
	{
		data->in_reg = true;
		PROC_CODE_LINE2(MOV, scalar_reg_arg,
			frmt("%d", typesize * data->capacity));
		PROC_CODE_LINE1(MUL, scalar_reg_arg);
		if (data->offset)
			// adding offset if there was any
			PROC_CODE_LINE2(ADD, eax_reg_arg,
				frmt("%d", data->offset)), data->offset = 0;
		addr_arg = frmt("dword ptr %s[ebp+eax]",
			data->entity->definition);
		if (IS_POINTER_TYPE(ltype))
		{
			PROC_CODE_LINE2(MOV, temp_reg_arg, frmt("dword ptr %s[ebp]",
				data->entity->definition));
			free(addr_arg), addr_arg = frmt("dword ptr [%s+eax]",
				temp_reg_arg);
		}
	}
	PROC_CODE_LINE2(LEA, temp_reg_arg, addr_arg);
	unreserve_register(REGISTERS, EAX);
	unreserve_register(REGISTERS, scalar_reg);

	data->dimension -= 1;
	data->capacity *= capacity;
	return data->type = ltype->base, data;
}

_addressable_data* gen_addressable_data_for_struct_accessor(
	_addressable_data* data, BinaryExpr* expr, StackFrame* frame)
{
	/*
		This function calculates and accumulates the offset between
		the struct's member relative to the beginning of this struct
	*/

	data->kind = ADDRESSABLE_ACCESSOR;
	// if its not struct type, then we cannot actually found the offset...
	if (!IS_STRUCT_OR_UNION_TYPE(data->type))
		report_error("Cannot find offset of member in not struct or union type."
			"in gen...data_for_struct_accessor()", NULL);
	for (size_t i = 0; i < sbuffer_len(data->type->members); i++)
	{
		if (strcmp(data->type->members[i]->name, expr->rexpr->idnt->svalue) == 0)
		{
			//todo: test this
			// if the specified type is union, we won't add any offset (its not tested yet)
			data->offset += !IS_UNION_TYPE(data->type) ?
				data->type->members[i]->offset : 0;
			return data->type = data->type->members[i]->type, data;
		}
	}

	// if we appeared in this code flow it means that ther was no 
	// struct member found, potentially it is not possible because
	// this issue is resolved in visitor
	report_error("There are no corresponding struct member found,"
		" it is a bug actually. in gen...data_for_struct_accessor().", NULL);
	return NULL;
}

_addressable_data* gen_addressable_data_for_struct_ptr_accessor(
	_addressable_data* data, BinaryExpr* expr, StackFrame* frame)
{
	/*
		Function calculates the offset for -> accessor,
		first, it calculates it relative to local variable using ebp register (if its not
		located in temp register), then it will calculate it based on reserved temp register
		all other accessor function will process that
	*/

	char* addr_reg_arg = NULL;
	// caching the current data->offset, 
	// because we will change it in nearest for-loop
	uint32_t cached_offset = data->offset;
	data->kind = ADDRESSABLE_PTR_ACCESSOR;

	// getting true type of variable (not pointer)
	// need only one dereference, because -> operator only works
	// with single pointer values by the left side
	Type* type = data->type = dereference_type(data->type);
	for (size_t i = 0; i < sbuffer_len(type->members); i++)
	{
		if (strcmp(type->members[i]->name, expr->rexpr->idnt->svalue) == 0)
		{
			// recaching the offset because now it will be relative to address
			// stored in address register
			data->type = type->members[i]->type;
			data->offset = type->members[i]->offset;
			break;
		}
	}

	// if we met the '->' operator,
	// it means that we need to get value stored in address
	// which will vary according in_reg flag
	// if itss true, it means that the needed address aleady stored in reg,
	// and no need to calculate offsets based on stack variable, need to calculate it based on this reg
	if (!data->in_reg)
		data->reg = get_unreserved_register(REGISTERS, REGSIZE_DWORD);
	addr_reg_arg = get_register_str(data->reg);
	if (data->in_reg)
		PROC_CODE_LINE2(MOV, addr_reg_arg,
			frmt("dword ptr [%s]", addr_reg_arg));
	else
	{
		data->in_reg = true;
		// just to make asm more clearly (without useless +0)
		char* addr_with_cached_offset = NULL;
		if (!cached_offset)
			addr_with_cached_offset = frmt("dword ptr %s[ebp]",
				data->entity->definition);
		else
			addr_with_cached_offset = frmt("dword ptr %s[ebp+%d]",
				data->entity->definition, cached_offset);
		PROC_CODE_LINE2(MOV, addr_reg_arg, addr_with_cached_offset);
	}
	// adding the actual offset for a member
	if (data->offset)
		PROC_CODE_LINE2(ADD, addr_reg_arg,
			frmt("%d", data->offset));
	return data;
}