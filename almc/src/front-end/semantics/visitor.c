#include "visitor.h"

#define IS_USER_TYPE_ALREADY_DECLARED(typedec, typestr) \
	if (is_##typedec##_declared(stmts[i]->type_decl->##typedec##_decl->##typedec##_name, table)) \
		report_error(frmt("\'%s\' type \'%s\' is already declared.",						             \
			typestr, stmts[i]->type_decl->##typedec##_decl->##typedec##_name), NULL);		     \
	add_##typedec##(stmts[i]->type_decl->##typedec##_decl, table);							     \

Visitor* visitor_new()
{
	Visitor* visitor = new_s(Visitor, visitor);
	visitor->global = table_new(NULL);
	return visitor;
}

void visitor_free(Visitor* visitor)
{
	if (visitor)
	{
		table_free(visitor->global);
		free(visitor);
	}
}

void visit(AstRoot* ast, Visitor* visitor)
{
	visit_scope(ast->stmts, visitor->global);
	for (size_t i = 0; i < sbuffer_len(ast->stmts); i++)
		visit_stmt(ast->stmts[i], visitor->global);
}

void visit_stmt(Stmt* stmt, Table* table)
{
	switch (stmt->kind)
	{
	case STMT_IF:
		visit_if_stmt(stmt->if_stmt, table);
		break;
	case STMT_LOOP:
		visit_loop_stmt(stmt->loop_stmt, table);
		break;
	case STMT_JUMP:
		visit_jump_stmt(stmt->jump_stmt, table);
		break;
	case STMT_EXPR:
		visit_expr(stmt->expr_stmt->expr, table);
		break;
	case STMT_BLOCK:
		visit_block(stmt->block, table_new(table));
		break;
	case STMT_SWITCH:
		visit_switch_stmt(stmt->switch_stmt, table);
		break;
	case STMT_VAR_DECL:
		visit_var_decl_stmt(stmt->var_decl, table);
		break;
	case STMT_TYPE_DECL:
		visit_type_decl_stmt(stmt->type_decl, table);
		break;
	case STMT_FUNC_DECL:
		visit_func_decl_stmt(stmt->func_decl, table);
		break;
	case STMT_LABEL_DECL:
		visit_label_decl_stmt(stmt->label_decl, table);
		break;
	case STMT_EMPTY:
		break;
	case STMT_IMPORT:
		visit_import_stmt(stmt->import_stmt, table);
		break;
	default:
		report_error("Unknown statement kind to visit in visit_stmt().", NULL);
	}
}

void visit_type(Type* type, Table* table)
{
	if (!type->mods.is_predefined)
		if (!is_struct_declared(type->repr, table) &&
			!is_union_declared(type->repr, table) &&
			!is_enum_declared(type->repr, table))
				report_error2(frmt("Undefined type \'%s\' met.",
					type->repr), type->area);
	if (type->mods.is_array && type->mods.is_ptr)
		report_error("Cannot resolve type ambiguity in visit_type()", NULL);
}

void visit_non_void_type(Type* type, Table* table)
{
	if (IS_VOID_TYPE(type))
		report_error2("Void type is not allowed in this context.",
			type->area);
	visit_type(type, table);
}

void visit_scope(Stmt** stmts, Table* table)
{
	for (size_t i = 0; i < sbuffer_len(stmts); i++)
	{
		switch (stmts[i]->kind)
		{
		case STMT_TYPE_DECL:
			switch (stmts[i]->type_decl->kind)
			{
			case TYPE_DECL_ENUM:
				IS_USER_TYPE_ALREADY_DECLARED(enum, "Enum");
				break;
			case TYPE_DECL_UNION:
				IS_USER_TYPE_ALREADY_DECLARED(union, "Union");
				break;
			case TYPE_DECL_STRUCT:
				IS_USER_TYPE_ALREADY_DECLARED(struct, "Struct");
				break;
			}
			break;
		case STMT_FUNC_DECL:
			if (is_function_declared(stmts[i]->func_decl->func_name->svalue, table))
				report_error(frmt("Function \'%s\' is already declared.",
					stmts[i]->func_decl->func_name->svalue), 
						stmts[i]->func_decl->func_name->context);
			add_function(stmts[i]->func_decl, table);
			break;
		// also added labels here
		case STMT_LABEL_DECL:
			if (is_label_declared(stmts[i]->label_decl->label_idnt->svalue, table))
				report_error(frmt("Label \'%s\' is already declared.",
					stmts[i]->label_decl->label_idnt->svalue), 
						stmts[i]->label_decl->label_idnt->context);
			add_label(stmts[i]->label_decl, table);
			break;
		}
	}
}

void visit_expr(Expr* expr, Table* table)
{
	switch (expr->kind)
	{
	case EXPR_CONST:
	case EXPR_STRING:
		break;
	case EXPR_IDNT:
		visit_idnt(expr->idnt, table, 0);
		break;
	case EXPR_FUNC_CALL:
		visit_func_call(expr->func_call, table);
		break;
	case EXPR_UNARY_EXPR:
		visit_unary_expr(expr->unary_expr, table);
		break;
	case EXPR_BINARY_EXPR:
		visit_binary_expr(expr->binary_expr, table);
		break;
	case EXPR_TERNARY_EXPR:
		visit_ternary_expr(expr->ternary_expr, table);
		break;
	case EXPR_INITIALIZER:
		report_error2("Initializers are not supported in language yet.", 
			expr->initializer->area);
		break;
	default:
		report_error("Unknown kind of binary expression met in visit_expr()", NULL);
	}
	get_and_set_expr_type(expr, table);
}

void visit_idnt(Idnt* idnt, Table* table, int is_in_assign)
{
	// checking if identifier is not a function parameter
	if (!is_enum_member(idnt->svalue, table) && 
		!is_function_param_passed(idnt->svalue, table))
	{
		// and check if declared and initialized
		if (!is_variable_declared(idnt->svalue, table))
			report_error(frmt("Variable \'%s\' is not declared.",
				idnt->svalue), idnt->context);
		if (is_in_assign && !is_variable_initialized(idnt->svalue, table))
			add_initialized_variable(idnt->svalue, table);
		if (!is_variable_initialized(idnt->svalue, table))
			report_error(frmt("Variable \'%s\' is not initialized in current scope.",
				idnt->svalue), idnt->context);
	}
}

void visit_func_call(FuncCall* func_call, Table* table)
{
	if (!is_function_declared(func_call->func_name, table))
		report_error2(frmt("Function \'%s\' is not declared in current scope.", 
			func_call->func_name), func_call->area);
	else
	{
		// visiting passed arguments to function call
		FuncDecl* origin = get_function(func_call->func_name, table);
		visit_type(origin->func_type, table);
		size_t passed   = sbuffer_len(func_call->func_args);
		size_t expected = sbuffer_len(origin->func_params);

		if (passed > expected)
			report_error2(frmt("Too much arguments passed to function call \'%s\'.", 
				func_call->func_name), func_call->area);

		if (passed < expected)
			report_error2(frmt("Not enough arguments passed to function call \'%s\'.", 
				func_call->func_name), func_call->area);
	}
}

void visit_unary_expr(UnaryExpr* unary_expr, Table* table)
{
	switch (unary_expr->kind)
	{
	case UNARY_CAST:
		visit_non_void_type(unary_expr->cast_type, table);
		visit_expr(unary_expr->expr, table);
		break;

	case UNARY_PLUS:
	case UNARY_MINUS:
	case UNARY_LG_NOT:
	case UNARY_BW_NOT:
	case UNARY_DEREFERENCE:
		visit_expr(unary_expr->expr, table);
		break;

	case UNARY_ADDRESS:
	case UNARY_PREFIX_INC:
	case UNARY_PREFIX_DEC:
	case UNARY_POSTFIX_INC:
	case UNARY_POSTFIX_DEC:
		if (!is_addressable_value(unary_expr->expr, table))
			report_error2("Addressable value expected for this unary operator.", 
				get_expr_area(unary_expr->expr));
		if (is_enum_member(unary_expr->expr->idnt->svalue, table))
			report_error2("Cannot use enum member with this unary operator.",
				get_expr_area(unary_expr->expr));
		visit_expr(unary_expr->expr, table);
		break;
	case UNARY_SIZEOF:
		visit_non_void_type(unary_expr->cast_type, table);
		break;
	case UNARY_LENGTHOF:
		visit_expr(unary_expr->expr, table);
		break;
	default:
		report_error("Unknown kind of unary expression met in visit_unary_expr()", NULL);
	}
}

void visit_binary_expr(BinaryExpr* binary_expr, Table* table)
{
	switch (binary_expr->kind)
	{
	case BINARY_ADD:
	case BINARY_SUB:
	case BINARY_DIV:
	case BINARY_MOD:
	case BINARY_MULT:
	case BINARY_COMMA:
	case BINARY_LSHIFT:
	case BINARY_RSHIFT:

	case BINARY_LG_OR:
	case BINARY_LG_AND:
	case BINARY_LG_EQ:
	case BINARY_LG_NEQ:

	case BINARY_BW_OR:
	case BINARY_BW_AND:
	case BINARY_BW_XOR:

	case BINARY_LESS_THAN:
	case BINARY_GREATER_THAN:
	case BINARY_LESS_EQ_THAN:
	case BINARY_GREATER_EQ_THAN:

	case BINARY_ARR_MEMBER_ACCESSOR:
		visit_expr(binary_expr->lexpr, table);
		visit_expr(binary_expr->rexpr, table);
		break;

	case BINARY_MEMBER_ACCESSOR:
	case BINARY_PTR_MEMBER_ACCESSOR:
		break;

	case BINARY_ASSIGN:
		if (!is_addressable_value(binary_expr->lexpr, table))
			report_error2("Cannot assign something to non-addressible value.",
				get_expr_area(binary_expr->lexpr));
		// if right expression is idnt, then set it as initialized
		if (binary_expr->lexpr->kind == EXPR_IDNT)
			visit_idnt(binary_expr->lexpr->idnt, table, 1);
		else 
			visit_expr(binary_expr->lexpr, table);
		visit_expr(binary_expr->rexpr, table);
		break;

	case BINARY_ADD_ASSIGN:
	case BINARY_SUB_ASSIGN:
	case BINARY_MUL_ASSIGN:
	case BINARY_DIV_ASSIGN:
	case BINARY_MOD_ASSIGN:
	case BINARY_LSHIFT_ASSIGN:
	case BINARY_RSHIFT_ASSIGN:
		if (!is_addressable_value(binary_expr->lexpr, table))
			report_error2("Cannot assign something to non-addressible value.", 
				get_expr_area(binary_expr->lexpr));
		visit_expr(binary_expr->lexpr, table);
		visit_expr(binary_expr->rexpr, table);
		break;
	default:
		report_error("Unknown kind of binary expression met in visit_binary_expr()", NULL);
	}
}

void visit_ternary_expr(TernaryExpr* ternary_expr, Table* table)
{
	visit_condition(ternary_expr->cond, table);
	visit_expr(ternary_expr->lexpr, table);
	visit_expr(ternary_expr->rexpr, table);
}

void visit_condition(Expr* condition, Table* table)
{
	visit_expr(condition, table);
	Type* type = get_expr_type(condition, table);
	if (!IS_NUMERIC_TYPE(type))
		report_error2(frmt("Condition expression must have numeric type, not \'%s\'",
			type_tostr_plain(type)), get_expr_area(condition));
}

void visit_if_stmt(IfStmt* if_stmt, Table* table)
{
	Table* local = NULL;
	visit_condition(if_stmt->if_cond, table);
	visit_scope(if_stmt->if_body->stmts, local = table_new(table));
	visit_block(if_stmt->if_body, local);

	for (size_t i = 0; i < sbuffer_len(if_stmt->elifs); i++)
		visit_condition(if_stmt->elifs[i]->elif_cond, table),
			visit_scope(if_stmt->elifs[i]->elif_body->stmts, local = table_new(table)),
				visit_block(if_stmt->elifs[i]->elif_body, local);

	if (if_stmt->else_body)
		visit_scope(if_stmt->else_body->stmts, local = table_new(table)),
			visit_block(if_stmt->else_body, local);
}

void check_for_conjuction_collisions(SwitchStmt* switch_stmt)
{
	int collision_resolved = 1;
	int cases_count = sbuffer_len(switch_stmt->switch_cases);
	for (size_t i = 0; i < cases_count; i++)
		collision_resolved = !switch_stmt->switch_cases[i]->is_conjucted;
	if (!collision_resolved)
		report_error2("Expected body for this case statement.",
			get_expr_area(switch_stmt->switch_cases[cases_count-1]->case_value));
}

void check_for_duplicated_case_conditions(SwitchStmt* switch_stmt, Table* table)
{
	Expr** conditions = NULL;

	for (size_t i = 0; i < sbuffer_len(switch_stmt->switch_cases); i++)
	{
		Expr* case_value = switch_stmt->switch_cases[i]->case_value;
		switch (case_value->kind)
		{
		case EXPR_IDNT:
			if (!is_enum_member(case_value->idnt->svalue, table))
				report_error2("Only enum members is allowed for case condition.",
					get_expr_area(case_value));
			for (size_t j = 0; j < sbuffer_len(conditions); j++)
				if (conditions[j]->kind == EXPR_IDNT)
					if (strcmp(conditions[j]->idnt->svalue, case_value->idnt->svalue) == 0)
						report_error2(frmt("This condition \'%s\' is already mentioned above.",
							case_value->idnt->svalue), get_expr_area(case_value));
			sbuffer_add(conditions, case_value);
			break;
		case EXPR_CONST:
			for (size_t j = 0; j < sbuffer_len(conditions); j++)
				if (conditions[j]->kind == EXPR_CONST)
					if (conditions[j]->cnst->ivalue == case_value->cnst->ivalue)
						report_error2(frmt("This condition \'%d\' is already mentioned above.",
							case_value->cnst->ivalue), get_expr_area(case_value));
			sbuffer_add(conditions, case_value);
			break;
		}
	}
	sbuffer_free(conditions);
}

void visit_switch_stmt(SwitchStmt* switch_stmt, Table* table)
{
	Table* local = NULL;
	visit_condition(switch_stmt->switch_cond, table);

	Type* switch_case_type = NULL;
	Type* switch_cond_type = get_expr_type(switch_stmt->switch_cond, table);

	if (!IS_INTEGRAL_TYPE(switch_cond_type))
		report_error2(frmt("Condition of switch statement must be of integral type, not \'%s\'", 
			type_tostr_plain(switch_cond_type)), get_expr_area(switch_stmt->switch_cond));

	check_for_conjuction_collisions(switch_stmt, table);
	check_for_duplicated_case_conditions(switch_stmt, table);

	for (size_t i = 0; i < sbuffer_len(switch_stmt->switch_cases); i++)
	{
		local = table_new(table);
		local->in_switch = switch_stmt;

		if (switch_stmt->switch_cases[i]->case_body)
			visit_scope(switch_stmt->switch_cases[i]->case_body->stmts, local),
				visit_block(switch_stmt->switch_cases[i]->case_body, local);

		switch_case_type = get_expr_type(switch_stmt->switch_cases[i]->case_value, local);
		if (!can_cast_implicitly(switch_cond_type, switch_case_type, local))
			report_error2(frmt("Cannot use case statement with type \'%s\' when switch's condition has type \'%s\'",
				type_tostr_plain(switch_case_type), type_tostr_plain(switch_cond_type)),
					get_expr_area(switch_stmt->switch_cases[i]->case_value));
		if (!IS_INTEGRAL_TYPE(switch_case_type))
			report_error2(frmt("Condition of case statement must be of integral type, not \'%s\'",
				type_tostr_plain(switch_case_type)), get_expr_area(switch_stmt->switch_cases[i]->case_value));
	}

	if (switch_stmt->switch_default)
	{
		local = table_new(table);
		local->in_switch = switch_stmt;
		visit_scope(switch_stmt->switch_default->stmts, local),
			visit_block(switch_stmt->switch_default, local);
	}
}

void visit_loop_stmt(LoopStmt* loop_stmt, Table* table)
{
	Table* local = table_new(table);
	local->in_loop = loop_stmt;
	switch (loop_stmt->kind)
	{
	case LOOP_DO:
		visit_do_loop_stmt(loop_stmt->do_loop, local);
		break;
	case LOOP_FOR:
		visit_for_loop_stmt(loop_stmt->for_loop, local);
		break;
	case LOOP_WHILE:
		visit_while_loop_stmt(loop_stmt->while_loop, local);
		break;
	default:
		report_error("Unknown loop kind met in visit_loop_stmt()",
			NULL);
	}
}

void visit_do_loop_stmt(DoLoop* do_loop, Table* table)
{
	visit_block(do_loop->do_body, table);
	visit_scope(do_loop->do_body->stmts, table);
	visit_condition(do_loop->do_cond, table);
}

void visit_for_loop_stmt(ForLoop* for_loop, Table* table)
{
	if (for_loop->for_init)
		visit_var_decl_stmt(for_loop->for_init, table);
	if (for_loop->for_cond)
		visit_condition(for_loop->for_cond, table);
	if (for_loop->for_step)
		visit_expr(for_loop->for_step, table);
	visit_scope(for_loop->for_body->stmts, table);
	visit_block(for_loop->for_body, table);
}

void visit_while_loop_stmt(WhileLoop* while_loop, Table* table)
{
	visit_condition(while_loop->while_cond, table);
	visit_scope(while_loop->while_body->stmts, table);
	visit_block(while_loop->while_body, table);
}

void visit_jump_stmt(JumpStmt* jump_stmt, Table* table)
{
	switch (jump_stmt->kind)
	{
	case JUMP_GOTO:
		visit_goto_stmt(jump_stmt, table);
		break;
	case JUMP_BREAK:
		visit_break_stmt(jump_stmt, table);
		break;
	case JUMP_RETURN:
		visit_return_stmt(jump_stmt, table);
		break;
	case JUMP_CONTINUE:
		visit_continue_stmt(jump_stmt, table);
		break;
	default:
		report_error("Unknown jump statement kind met in visit_jump_stmt()", 
			NULL);
	}
}

void visit_goto_stmt(JumpStmt* goto_stmt, Table* table)
{
	if (!goto_stmt->additional_expr)
		report_error2("Expression in jump statement is undefined in visit_goto_stmt().", NULL);
	else
	{
		if (goto_stmt->additional_expr->kind != EXPR_IDNT)
			report_error2("Expression in jump statement must be identifier in case of goto statement.",
				goto_stmt->area);
		if (!is_label_declared(goto_stmt->additional_expr->idnt->svalue, table))
			report_error2(frmt("Label \'%s\' is not declared in current scope.",
				goto_stmt->additional_expr->idnt->svalue), goto_stmt->area);
	}
}

void visit_break_stmt(JumpStmt* break_stmt, Table* table)
{
	if (!table->in_loop && !table->in_switch)
		report_error2("Cannot use break statement in this context.", 
			break_stmt->area);
}

void visit_return_stmt(JumpStmt* return_stmt, Table* table)
{
	if (!table->in_function)
		report_error2("Cannot use return statement when its not located in function.",
			return_stmt->area);
	else
	{
		if (!IS_VOID_TYPE(table->in_function->func_type))
		{
			if (!return_stmt->additional_expr)
				report_error2("Return statement must return some value from function.",
					return_stmt->area);
			visit_expr(return_stmt->additional_expr, table);
			Type* return_type = get_expr_type(return_stmt->additional_expr, table);
			if (!can_cast_implicitly(table->in_function->func_type, return_type))
				report_error2(frmt("Cannot return value of type \'%s\' from function with \'%s\'.",
					type_tostr_plain(return_type), type_tostr_plain(table->in_function->func_type)),
						get_expr_area(return_stmt->additional_expr));
		}
		else
			if (return_stmt->additional_expr)
				report_error2("Cannot return value from function with void type.",
					get_expr_area(return_stmt->additional_expr));
	}
}

void visit_continue_stmt(JumpStmt* continue_stmt, Table* table)
{
	if (!table->in_loop)
		report_error2("Cannot use continue statement in this context.", 
			continue_stmt->area);
}

void visit_var_decl_stmt(VarDecl* var_decl, Table* table)
{
	Type* type = var_decl->type_var->type;
	SrcArea* area = var_decl->type_var->area;
	const char* var = var_decl->type_var->var;

	if (is_variable_declared(var, table))
		report_error2(frmt("Variable \'%s\' is already declared.",
			var), area);
	else if (is_function_param_passed(var, table))
		report_error2(frmt("\'%s\' is already declared as function's parameter.",
			var), area);
	else if (is_enum_member(var, table))
		report_error2(frmt("\'%s\' is already declared as enum's identifier.",
			var), area);
	else
	{
		add_variable(var_decl, table);
		visit_non_void_type(type, table);
		if (var_decl->var_init)
		{
			visit_expr(var_decl->var_init, table);
			add_initialized_variable(var, table);

			// check type of created variable with type of initializing expression
			Type* init_expr_type = get_expr_type(var_decl->var_init, table);
			if (!can_cast_implicitly(type, init_expr_type))
				report_error2(frmt("Expression-initializer has incompatible type \'%s\' with type of variable \'%s\'.",
					type_tostr_plain(init_expr_type), type_tostr_plain(type)), 
						get_expr_area(var_decl->var_init));
		}
	}
}

void visit_enum(EnumDecl* enum_decl, Table* table)
{
	for (size_t i = 0; i < sbuffer_len(enum_decl->enum_idnts); i++)
	{
		// checking validity of values that assigned to enum idents
		for (size_t j = 0; j < sbuffer_len(enum_decl->enum_idnt_values); j++)
			if (!is_simple_const_expr(enum_decl->enum_idnt_values[j]))
				report_error2(frmt("Enum member \'%s\' must have constant expression, in \'%s\' enum.",
					enum_decl->enum_idnts[j]->svalue, enum_decl->enum_name),
						get_expr_area(enum_decl->enum_idnt_values[j]));

		// iterating through all members of all enums (except current enum)
		for (Table* parent = table; parent != NULL; parent = parent->parent)                        // each scope from above scope
			for (size_t j = 0; j < sbuffer_len(parent->enums); j++)                                 // each enum in iterating scope
				if (strcmp(parent->enums[j]->enum_name, enum_decl->enum_name) != 0)                 // if iterating enum != current enum
					for (size_t z = 0; z < sbuffer_len(parent->enums[j]->enum_idnts); z++)          // each member in iterating enum
						if (strcmp(parent->enums[j]->enum_idnts[z]->svalue,
							enum_decl->enum_idnts[i]->svalue) == 0)
								report_error(frmt("Enum member \'%s\' is already declared in \'%s\' enum.",
									enum_decl->enum_idnts[i]->svalue, parent->enums[j]->enum_name),
										enum_decl->enum_idnts[i]->context);
		// checking value type
		
		// evaluating the value to get the proper type of it
		// and then compare with 8 byte type
		// enum identifier's value must be less equal than 4 bytes
		Type* const_expr_type = get_ivalue_type(
			evaluate_expr_itype(enum_decl->enum_idnt_values[i]));
		if (get_type_size_in_bytes(const_expr_type) > 4)
			report_error2(frmt("Enum member \'%s\' must have value type less equal than 4 bytes, in \'%s\' enum.",
				enum_decl->enum_idnts[i]->svalue, enum_decl->enum_name),
					get_expr_area(enum_decl->enum_idnt_values[i]));
		type_free(const_expr_type);

		// set type to enum identifier
		Type* value_type = get_expr_type(enum_decl->enum_idnt_values[i], table);
		if (!IS_INTEGRAL_TYPE(value_type))
			report_error2(frmt("Enum's member \'%s\' has incompatible \'%s\' type in \'%s\' enum.",
				enum_decl->enum_idnts[i]->svalue, type_tostr_plain(value_type), enum_decl->enum_name), 
					get_expr_area(enum_decl->enum_idnt_values[i]));

		// checking for any duplicated names in current enum
		for (size_t j = i + 1; j < sbuffer_len(enum_decl->enum_idnts); j++)
			if (strcmp(enum_decl->enum_idnts[i]->svalue, enum_decl->enum_idnts[j]->svalue) == 0)
				report_error(frmt("Member \'%s\' is already declared in \'%s\' enum declaration.",
					enum_decl->enum_idnts[i]->svalue, enum_decl->enum_name), enum_decl->enum_idnts[i]->context);
	}
}

void visit_union(UnionDecl* union_decl, Table* table)
{
	for (size_t i = 0; i < sbuffer_len(union_decl->union_mmbrs); i++)
	{
		// checking for self included type (and not pointer)
		if (strcmp(union_decl->union_name, union_decl->union_mmbrs[i]->type->repr) == 0)
			if (!union_decl->union_mmbrs[i]->type->mods.is_ptr)
				report_error2(frmt("Member \'%s\' has type \'%s\' which is self included, and not pointer.",
					union_decl->union_mmbrs[i]->var, union_decl->union_name), 
						union_decl->union_mmbrs[i]->area);
		visit_non_void_type(union_decl->union_mmbrs[i]->type, table);
	}

	// checking here for duplicated members
	for (size_t i = 0; i < sbuffer_len(union_decl->union_mmbrs); i++)
		for (size_t j = i + 1; j < sbuffer_len(union_decl->union_mmbrs); j++)
			if (strcmp(union_decl->union_mmbrs[i]->var, union_decl->union_mmbrs[j]->var) == 0)
				report_error2(frmt("Member \'%s\' is already declared in \'%s\' union declaration.",
					union_decl->union_mmbrs[i]->var, union_decl->union_name), union_decl->union_mmbrs[i]->area);
}

void visit_struct(StructDecl* struct_decl, Table* table)
{
	// checking all member's types
	for (size_t i = 0; i < sbuffer_len(struct_decl->struct_mmbrs); i++)
	{
		// checking for self included type (and not pointer)
		if (strcmp(struct_decl->struct_name, struct_decl->struct_mmbrs[i]->type->repr) == 0)
			if (!struct_decl->struct_mmbrs[i]->type->mods.is_ptr)
				report_error2(frmt("Member \'%s\' has type \'%s\' which is self included, and not pointer.",
					struct_decl->struct_mmbrs[i]->var, struct_decl->struct_name), 
						struct_decl->struct_mmbrs[i]->area);
		visit_non_void_type(struct_decl->struct_mmbrs[i]->type, table);
	}

	// checking here for duplicated members
	for (size_t i = 0; i < sbuffer_len(struct_decl->struct_mmbrs); i++)
		for (size_t j = i+1; j < sbuffer_len(struct_decl->struct_mmbrs); j++)
			if (strcmp(struct_decl->struct_mmbrs[i]->var, struct_decl->struct_mmbrs[j]->var) == 0)
				report_error2(frmt("Member \'%s\' is already declared in \'%s\' struct declaration.",
					struct_decl->struct_mmbrs[i]->var, struct_decl->struct_name), 
						struct_decl->struct_mmbrs[i]->area);
}

void visit_type_decl_stmt(TypeDecl* type_decl, Table* table)
{
	Table* local = table_new(NULL);
	switch (type_decl->kind)
	{
	case TYPE_DECL_ENUM:
		visit_enum(type_decl->enum_decl, table);
		break;
	case TYPE_DECL_UNION:
		visit_union(type_decl->union_decl, table);
		break;
	case TYPE_DECL_STRUCT:
		visit_struct(type_decl->struct_decl, table);
		break;
	default:
		report_error("Unknown type of type declaration met in visit_type_decl_stmt()", NULL);
	}
}

void visit_block(Block* block, Table* table)
{
	for (size_t i = 0; i < sbuffer_len(block->stmts); i++)
		visit_stmt(block->stmts[i], table);
}

void visit_func_decl_stmt(FuncDecl* func_decl, Table* table)
{
	Table* local = table_new(table);
	local->in_function = func_decl;

	if (func_decl->func_spec.is_intrinsic)
		report_error("Intrinsic specifier is not supported in language yet.", 
			func_decl->func_name->context);

	if (func_decl->func_spec.is_entry)
		visit_entry_func_stmt(func_decl, table);

	if (func_decl->func_body)
		visit_scope(func_decl->func_body->stmts, local);

	// checking func parameters
	for (size_t i = 0; i < sbuffer_len(func_decl->func_params); i++)
	{
		// checking if parameter with this name is already defined as enum identifier
		if (is_enum_member(func_decl->func_params[i]->var, local))
			report_error2(frmt("Function parameter \'%s\' is already defined as enum identifier.",
				func_decl->func_params[i]->var), func_decl->func_params[i]->area);
		// checking for duplicated parameter specification
		if (is_function_param_passed(func_decl->func_params[i]->var, local))
			report_error2(frmt("Function parameter \'%s\' is already specified in \'%s\' function's parameters.",
				func_decl->func_params[i]->var, func_decl->func_name->svalue), func_decl->func_params[i]->area);
		add_function_param(func_decl->func_params[i], local);

		// checking for void type
		visit_non_void_type(func_decl->func_params[i]->type, table);
	}

	// checking function's return type
	visit_type(func_decl->func_type, table);
	// checking function's body
	visit_block(func_decl->func_body, local);
	//checking that all code paths return value
	check_func_return_flow(func_decl);
}

void visit_label_decl_stmt(LabelDecl* label_decl, Table* table)
{
	// there are no any processing stuff for label declaration statement yet.  ? ?
	// check for duplicated label is already exists in visit_scope
}

void visit_import_stmt(ImportStmt* import_stmt, Table* table)
{
	visit_scope(import_stmt->imported_ast->stmts, table);
	for (size_t i = 0; i < sbuffer_len(import_stmt->imported_ast->stmts); i++)
		visit_stmt(import_stmt->imported_ast->stmts[i], table);
}

void check_entry_func_params(FuncDecl* func_decl)
{
	TypeVar** params = func_decl->func_params;
	size_t param_count = sbuffer_len(params);

	switch (param_count)
	{
	case 0:
		break;
	case 2:
		if (!IS_I32_TYPE(params[0]->type))
			report_error2("First parameter of an entry method"
				" should be of type \'i32\' in this context.", params[0]->type->area);
		if (!IS_CHAR_POINTER_TYPE(params[1]->type) ||
			!IS_POINTER_RANK(2, params[1]->type))
			report_error2("Second parameter of an entry method should"
				" be of type \'char**\' in this context.", params[1]->type->area);
		break;
	default:
		report_error(frmt("Entry method \'%s\' cannot accept this count \'%d\' of parameters.",
			func_decl->func_name->svalue, param_count), func_decl->func_name->context);
	}
}

void visit_entry_func_stmt(FuncDecl* func_decl, Table* table)
{
	if (func_decl->func_spec.is_entry && table->parent)
		report_error(frmt("Cannot create entry function \'%s\' not in global scope.",
			func_decl->func_name->svalue), func_decl->func_name->context);

	if (func_decl->func_spec.is_entry)
		for (size_t i = 0; i < sbuffer_len(table->functions); i++)
			if (table->functions[i]->func_spec.is_entry &&
				table->functions[i] != func_decl)
				report_error(frmt("Cannot specify function \'%s\' as entry,"
					" entry function \'%s\' is already mentioned.", func_decl->func_name->svalue, 
						table->functions[i]->func_name->svalue), func_decl->func_name->context);

	check_entry_func_params(func_decl);
}