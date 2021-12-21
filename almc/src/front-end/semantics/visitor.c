#include "visitor.h"

//todo: check type referrings to each other (for freeing)
//todo: add enum identifiers to is_const_expression

#define IS_USER_TYPE_ALREADY_DECLARED(typedec, typestr) \
	if (is_##typedec##_declared(stmts[i]->type_decl->##typedec##_decl->name, table)) \
		report_error(frmt("\'%s\' type \'%s\' is already declared.",				 \
			typestr, stmts[i]->type_decl->##typedec##_decl->name), NULL);		     \
	add_##typedec##(stmts[i]->type_decl->##typedec##_decl, table);					 \

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
	Type* base = get_base_type(type);
	if (!is_not_aggregate_type(base))
		if (!is_struct_declared(type->repr, table) &&
			!is_union_declared(type->repr, table) &&
			!is_enum_declared(type->repr, table))
				report_error2(frmt("Undefined type \'%s\' met.",
					type->repr), type->area);
	if (IS_ARRAY_TYPE(type))
		visit_expr(type->dimension, table), 
			visit_type(type->base, table);
	if (is_pointer_like_type(type))
		visit_pointer_like_type(type, table);
	if (IS_INCOMPLETE_TYPE(base))
		complete_type(type, table);
	complete_size(type, table);
}

void visit_pointer_like_type(Type* type, Table* table)
{
	switch (type->kind)
	{
	case TYPE_ARRAY:
		if (!is_const_expr(type->dimension, table))
			report_error2("Array size is not a constant expression.",
				get_expr_area(type->dimension));
		visit_pointer_like_type(type->base, table);
		break;
	}
}

void visit_non_void_type(Type* type, SrcArea* area, Table* table)
{
	/*
		SrcArea required here because void type is preallocated type with
		no self-area, so, need to specify it
	*/
	if (IS_VOID_TYPE(type))
		report_error2("void type is not allowed in this context.", area);
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
			if (is_function_declared(stmts[i]->func_decl->name->svalue, table))
				report_error(frmt("Function \'%s\' is already declared.",
					stmts[i]->func_decl->name->svalue), 
						stmts[i]->func_decl->name->context);
			add_function(stmts[i]->func_decl, table);
			break;
		// also added labels here
		case STMT_LABEL_DECL:
			if (is_label_declared(stmts[i]->label_decl->label->svalue, table))
				report_error(frmt("Label \'%s\' is already declared.",
					stmts[i]->label_decl->label->svalue), 
						stmts[i]->label_decl->label->context);
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
	if (is_enum_member(idnt->svalue, table))
		visit_enum_member(idnt, table);
	else
	{
		// checking if identifier is not a function parameter
		if (!is_function_param_passed(idnt->svalue, table))
		{
			// and check if declared and initialized
			if (!is_variable_declared(idnt->svalue, table))
				report_error(frmt("Variable \'%s\' is not declared.",
					idnt->svalue), idnt->context);
			if (is_in_assign && !is_variable_initialized(idnt->svalue, table))
				add_initialized_variable(idnt->svalue, table);

			//todo: comeback here later
			//if (!is_variable_initialized(idnt->svalue, table))
			//	report_error(frmt("Variable \'%s\' is not initialized in current scope.",
			//		idnt->svalue), idnt->context);
		}
	}

}

void visit_enum_member(Idnt* idnt, Table* table)
{
	Expr* value = NULL;

	for (Table* parent = table; parent; parent = parent->parent)
		for (size_t i = 0; i < sbuffer_len(parent->enums); i++)
			for (size_t j = 0; j < sbuffer_len(parent->enums[i]->members); j++)
				if (strcmp(idnt->svalue, parent->enums[i]->members[j]->name) == 0)
					value = parent->enums[i]->members[j]->value;
	if (value)
		idnt->is_enum_member = true,
			idnt->enum_member_value = value;
}

void visit_func_call(FuncCall* func_call, Table* table)
{
	if (!is_function_declared(func_call->name, table))
		report_error2(frmt("Function \'%s\' is not declared in current scope.", 
			func_call->name), func_call->area);
	else
	{
		FuncDecl* origin = get_function(
			func_call->name, table);
		visit_type(origin->type, table);
		func_call->conv = origin->conv;
		func_call->spec = origin->spec;

		// calculating count of params without vararg (...)
		// if its declared
		int32_t params = (int32_t)sbuffer_len(origin->params);

		int32_t args_passed = (int32_t)
			sbuffer_len(func_call->args);
		
		if (args_passed < params)
			report_error2(frmt("Not enough arguments passed to function call \'%s\'.",
				func_call->name), func_call->area);

		// if passed args count is greater than actual params
		// and if the origin function is not vararg
		else if (!origin->spec->is_vararg)
			if (args_passed > params)
				report_error2(frmt("Too much arguments passed to function call \'%s\'.",
					func_call->name), func_call->area);

		// visiting passed arguments to function call
		for (size_t i = 0; i < sbuffer_len(func_call->args); i++)
			visit_expr(func_call->args[i], table);
	}
}

void visit_unary_expr(UnaryExpr* unary_expr, Table* table)
{
	switch (unary_expr->kind)
	{
	case UNARY_CAST:
		visit_non_void_type(unary_expr->cast_type, unary_expr->area, table);
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
		visit_non_void_type(unary_expr->cast_type, unary_expr->area, table);
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
		visit_expr(binary_expr->lexpr, table);
		visit_expr(binary_expr->rexpr, table);
		break;

	case BINARY_ARR_MEMBER_ACCESSOR:
		visit_expr(binary_expr->lexpr, table);
		visit_expr(binary_expr->rexpr, table);
		visit_array_member_accessor(binary_expr, table);
		break;

	case BINARY_MEMBER_ACCESSOR:
	case BINARY_PTR_MEMBER_ACCESSOR:
		if (binary_expr->lexpr->kind != EXPR_IDNT)
			visit_expr(binary_expr->lexpr, table);
		else
			visit_idnt(binary_expr->lexpr->idnt, table, false);
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
	case BINARY_BW_OR_ASSIGN:
	case BINARY_BW_AND_ASSIGN:
	case BINARY_BW_XOR_ASSIGN:
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

void visit_array_member_accessor(BinaryExpr* arr_accessor_expr, Table* table)
{
	/*Type* expr_type = get_binary_expr_type(
		arr_accessor_expr, table);
	Expr* rexpr = arr_accessor_expr->rexpr;

	expr_type->capacity = evaluate_expr_itype(
		expr_type->dimension);

	// in this case we cannot evaluate expression, so do nothing
	if (!is_const_expr(rexpr, table))
		return;

	int32_t index = evaluate_expr_itype(rexpr);
	if (index >= expr_type->capacity)
		report_error2(frmt("Array accessor index does not fits in current dimension's capacity. (size: %d, index: %d)",
			expr_type->capacity, index), get_expr_area(rexpr));*/
}

void visit_condition(Expr* condition, Table* table)
{
	visit_expr(condition, table);
	Type* type = retrieve_expr_type(condition, table);
	if (!is_numeric_type(type) && !is_pointer_like_type(type))
		report_error2(frmt("Condition expression must have numeric type, not \'%s\'",
			type_tostr_plain(type)), get_expr_area(condition));
}

void visit_if_stmt(IfStmt* if_stmt, Table* table)
{
	Table* local = NULL;
	visit_condition(if_stmt->cond, table);
	visit_scope(if_stmt->body->stmts, local = table_new(table));
	visit_block(if_stmt->body, local);

	for (size_t i = 0; i < sbuffer_len(if_stmt->elifs); i++)
		visit_condition(if_stmt->elifs[i]->cond, table),
			visit_scope(if_stmt->elifs[i]->body->stmts, local = table_new(table)),
				visit_block(if_stmt->elifs[i]->body, local);

	if (if_stmt->else_body)
		visit_scope(if_stmt->else_body->stmts, local = table_new(table)),
			visit_block(if_stmt->else_body, local);
}

void check_for_conjuction_collisions(SwitchStmt* switch_stmt)
{
	int collision_resolved = 1;
	int cases_count = sbuffer_len(switch_stmt->cases);
	for (size_t i = 0; i < cases_count; i++)
		collision_resolved = !switch_stmt->cases[i]->is_conjucted;
	if (!collision_resolved)
		report_error2("Expected body for this case statement.",
			get_expr_area(switch_stmt->cases[cases_count-1]->value));
}

void check_for_duplicated_case_conditions(SwitchStmt* switch_stmt, Table* table)
{
	Expr** conditions = NULL;

	for (size_t i = 0; i < sbuffer_len(switch_stmt->cases); i++)
	{
		Expr* case_value = switch_stmt->cases[i]->value;
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
	visit_condition(switch_stmt->cond, table);

	Type* switch_case_type = NULL;
	Type* switch_cond_type = get_expr_type(switch_stmt->cond, table);

	if (!is_integral_type(switch_cond_type))
		report_error2(frmt("Condition of switch statement must be of integral type, not \'%s\'", 
			type_tostr_plain(switch_cond_type)), get_expr_area(switch_stmt->cond));

	check_for_conjuction_collisions(switch_stmt, table);
	check_for_duplicated_case_conditions(switch_stmt, table);

	for (size_t i = 0; i < sbuffer_len(switch_stmt->cases); i++)
	{
		local = table_new(table);
		local->in_switch = switch_stmt;

		if (switch_stmt->cases[i]->body)
			visit_scope(switch_stmt->cases[i]->body->stmts, local),
				visit_block(switch_stmt->cases[i]->body, local);

		switch_case_type = get_expr_type(switch_stmt->cases[i]->value, local);
		if (!can_cast_implicitly(switch_cond_type, switch_case_type, local))
			report_error2(frmt("Cannot use case statement with type \'%s\' when switch's condition has type \'%s\'",
				type_tostr_plain(switch_case_type), type_tostr_plain(switch_cond_type)),
					get_expr_area(switch_stmt->cases[i]->value));
		if (!is_integral_type(switch_case_type))
			report_error2(frmt("Condition of case statement must be of integral type, not \'%s\'",
				type_tostr_plain(switch_case_type)), get_expr_area(switch_stmt->cases[i]->value));
	}

	if (switch_stmt->default_case)
	{
		local = table_new(table);
		local->in_switch = switch_stmt;
		visit_scope(switch_stmt->default_case->stmts, local),
			visit_block(switch_stmt->default_case, local);
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
	visit_block(do_loop->body, table);
	visit_scope(do_loop->body->stmts, table);
	visit_condition(do_loop->cond, table);
}

void visit_for_loop_stmt(ForLoop* for_loop, Table* table)
{
	if (for_loop->init)
		visit_var_decl_stmt(for_loop->init, table);
	if (for_loop->cond)
		visit_condition(for_loop->cond, table);
	if (for_loop->step)
		visit_expr(for_loop->step, table);
	visit_scope(for_loop->body->stmts, table);
	visit_block(for_loop->body, table);
}

void visit_while_loop_stmt(WhileLoop* while_loop, Table* table)
{
	visit_condition(while_loop->cond, table);
	visit_scope(while_loop->body->stmts, table);
	visit_block(while_loop->body, table);
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
		if (!IS_VOID_TYPE(table->in_function->type))
		{
			if (!return_stmt->additional_expr)
				report_error2("Return statement must return some value from function.",
					return_stmt->area);
			visit_expr(return_stmt->additional_expr, table);
			Type* return_type = get_expr_type(return_stmt->additional_expr, table);
			if (!can_cast_implicitly(table->in_function->type, return_type))
				report_error2(frmt("Cannot return value of type \'%s\' from function with \'%s\'.",
					type_tostr_plain(return_type), type_tostr_plain(table->in_function->type)),
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
		visit_non_void_type(type, area, table);
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
	for (size_t i = 0; i < sbuffer_len(enum_decl->members); i++)
	{
		// checking validity of values that assigned to enum idents
		for (size_t j = 0; j < sbuffer_len(enum_decl->members); j++)
			if (!is_const_expr(enum_decl->members[j]->value, table))
				report_error2(frmt("Enum member \'%s\' must have constant expression, in \'%s\' enum.",
					enum_decl->members[j]->name, enum_decl->name),
						get_expr_area(enum_decl->members[j]->value));

		// iterating through all members of all enums (except current enum)
		for (Table* parent = table; parent != NULL; parent = parent->parent)                        // each scope from above scope
			for (size_t j = 0; j < sbuffer_len(parent->enums); j++)                                 // each enum in iterating scope
				if (strcmp(parent->enums[j]->name, enum_decl->name) != 0)                 // if iterating enum != current enum
					for (size_t z = 0; z < sbuffer_len(parent->enums[j]->members); z++)          // each member in iterating enum
						if (strcmp(parent->enums[j]->members[z]->name,
							enum_decl->members[i]->name) == 0)
								report_error(frmt("Enum member \'%s\' is already declared in \'%s\' enum.",
									enum_decl->members[i]->name, parent->enums[j]->name),
										enum_decl->members[i]->context);
		
		// set type to enum identifier
		Type* value_type = get_expr_type(enum_decl->members[i]->value, table);
		if (!is_integral_type(value_type) || value_type->size > i32_type.size)
			report_error2(frmt("Enum's member \'%s\' has incompatible \'%s\' type in \'%s\' enum.",
				enum_decl->members[i]->name, type_tostr_plain(value_type), enum_decl->name),
					get_expr_area(enum_decl->members[i]->value));

		// checking value type
		// evaluating the value to get the proper type of it
		// and then compare with 4 byte type
		// enum identifier's value must be less equal than 4 bytes
		Type* const_expr_type = get_ivalue_type(
			evaluate_expr_itype(enum_decl->members[i]->value));
		if (const_expr_type->size >= i32_type.size)
			report_error2(frmt("Enum member \'%s\' must have value type less equal than 4 bytes, in \'%s\' enum.",
				enum_decl->members[i]->name, enum_decl->name),
					get_expr_area(enum_decl->members[i]->value));

		// checking for any duplicated names in current enum
		for (size_t j = i + 1; j < sbuffer_len(enum_decl->members); j++)
			if (strcmp(enum_decl->members[i]->name, enum_decl->members[j]->name) == 0)
				report_error(frmt("Member \'%s\' is already declared in \'%s\' enum declaration.",
					enum_decl->members[i]->name, enum_decl->name), enum_decl->members[i]->context);
	}
}

void visit_union(UnionDecl* union_decl, Table* table)
{
	for (size_t i = 0; i < sbuffer_len(union_decl->members); i++)
	{
		// checking for self included type (and not pointer)
		if (strcmp(union_decl->name, union_decl->members[i]->type->repr) == 0)
			if (!IS_POINTER_TYPE(union_decl->members[i]->type))
				report_error2(frmt("Member \'%s\' has type \'%s\' which is self included, and not pointer.",
					union_decl->members[i]->name, union_decl->name),
						union_decl->members[i]->area);
		visit_non_void_type(union_decl->members[i]->type, 
			union_decl->members[i]->area, table);
	}

	// checking here for duplicated members
	for (size_t i = 0; i < sbuffer_len(union_decl->members); i++)
		for (size_t j = i + 1; j < sbuffer_len(union_decl->members); j++)
			if (strcmp(union_decl->members[i]->name, union_decl->members[j]->name) == 0)
				report_error2(frmt("Member \'%s\' is already declared in \'%s\' union declaration.",
					union_decl->members[i]->name, union_decl->name), union_decl->members[i]->area);
}

void visit_struct(StructDecl* struct_decl, Table* table)
{
	// checking all member's types
	for (size_t i = 0; i < sbuffer_len(struct_decl->members); i++)
	{
		// checking for self included type (and not pointer)
		if (strcmp(struct_decl->name, struct_decl->members[i]->type->repr) == 0)
			if (!IS_POINTER_TYPE(struct_decl->members[i]->type))
				report_error2(frmt("Member \'%s\' has type \'%s\' which is self included, and not pointer.",
					struct_decl->members[i]->name, struct_decl->name),
						struct_decl->members[i]->area);
		visit_non_void_type(struct_decl->members[i]->type,
			struct_decl->members[i]->area, table);
	}

	// checking here for duplicated members
	for (size_t i = 0; i < sbuffer_len(struct_decl->members); i++)
		for (size_t j = i+1; j < sbuffer_len(struct_decl->members); j++)
			if (strcmp(struct_decl->members[i]->name, struct_decl->members[j]->name) == 0)
				report_error2(frmt("Member \'%s\' is already declared in \'%s\' struct declaration.",
					struct_decl->members[i]->name, struct_decl->members),
						struct_decl->members[i]->area);
	visit_struct_members(struct_decl->members);
}

void visit_struct_members(Member** members)
{
	for (size_t i = 0; i < sbuffer_len(members); i++)
	{
		if (i == 0)
			members[i]->offset = 0;
		else
			members[i]->offset = members[i - 1]->offset +
				members[i - 1]->type->size;
	}
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

void visit_func_decl_calling_convention(FuncDecl* func_decl)
{
	switch (func_decl->conv->kind)
	{
	case CALL_CONV_CDECL:
		break;
	case CALL_CONV_STDCALL:
		if (func_decl->spec->is_vararg)
			report_error("Cannot use vararg with stdcall calling convention.",
				func_decl->name->context);
		break;
	}
}

bool visit_func_decl_specifiers(FuncDecl* func_decl, Table* table)
{
	if (func_decl->spec->is_entry)
		visit_entry_func_stmt(func_decl, table);

	visit_func_decl_calling_convention(func_decl);

	//todo: also check file
	if (func_decl->spec->is_external && func_decl->body)
		report_error(frmt("Function \'%s\' specified like external.",
			func_decl->name->svalue), func_decl->name->context);
	else if (!func_decl->spec->is_external && !func_decl->body)
		report_error(frmt("Function \'%s\' needs body, its not external.",
			func_decl->name->svalue), func_decl->name->context);

	return func_decl->spec->is_external;
}

void visit_func_decl_stmt(FuncDecl* func_decl, Table* table)
{
	Table* local = table_new(table);
	local->in_function = func_decl;

	if (!visit_func_decl_specifiers(func_decl, table))
		visit_scope(func_decl->body->stmts, local);

	// checking func parameters
	for (size_t i = 0; i < sbuffer_len(func_decl->params); i++)
	{
		// checking if parameter with this name is already defined as enum identifier
		if (is_enum_member(func_decl->params[i]->var, local))
			report_error2(frmt("Function parameter \'%s\' is already defined as enum identifier.",
				func_decl->params[i]->var), func_decl->params[i]->area);
		// checking for duplicated parameter specification
		if (is_function_param_passed(func_decl->params[i]->var, local))
			report_error2(frmt("Function parameter \'%s\' is already specified in \'%s\' function's parameters.",
				func_decl->params[i]->var, func_decl->name->svalue), func_decl->params[i]->area);
		add_function_param(func_decl->params[i], local);

		// checking func param for void type
		visit_non_void_type(func_decl->params[i]->type, 
			func_decl->params[i]->area, table);
	}

	// if function is external it does not have a body
	if (!func_decl->spec->is_external)
	{
		// checking function's return type
		visit_type(func_decl->type, table);
		// checking function's body
		visit_block(func_decl->body, local);
		//checking that all code paths return value
		check_func_return_flow(func_decl);
	}
}

void visit_label_decl_stmt(LabelDecl* label_decl, Table* table)
{
	// there are no any processing stuff for label declaration statement yet.  ? ?
	// check for duplicated label is already exists in visit_scope
}

void visit_import_stmt(ImportStmt* import_stmt, Table* table)
{
	visit_scope(import_stmt->ast->stmts, table);
	for (size_t i = 0; i < sbuffer_len(import_stmt->ast->stmts); i++)
		visit_stmt(import_stmt->ast->stmts[i], table);
}

void check_entry_func_params(FuncDecl* func_decl)
{
	TypeVar** params = func_decl->params;
	size_t param_count = sbuffer_len(params);

	switch (param_count)
	{
	case 0:
		break;
	case 2:
		if (!IS_I32_TYPE(params[0]->type))
			report_error2("First parameter of an entry method"
				" should be of type \'i32\' in this context.", params[0]->area);
		if (!IS_CHAR_TYPE(get_base_type(params[1]->type)) || get_pointer_rank(params[1]->type) != 2)
			report_error2("Second parameter of an entry method should"
				" be of type \'char**\' in this context.", params[1]->area);
		break;
	default:
		report_error(frmt("Entry method \'%s\' cannot accept this count \'%d\' of parameters.",
			func_decl->name->svalue, param_count), func_decl->name->context);
	}
}

void visit_entry_func_stmt(FuncDecl* func_decl, Table* table)
{
	if (func_decl->spec->is_entry && table->parent)
		report_error(frmt("Cannot create entry function \'%s\' not in global scope.",
			func_decl->name->svalue), func_decl->name->context);

	if (func_decl->spec->is_entry)
		for (size_t i = 0; i < sbuffer_len(table->functions); i++)
			if (table->functions[i]->spec->is_entry &&
				table->functions[i] != func_decl)
				report_error(frmt("Cannot specify function \'%s\' as entry,"
					" entry function \'%s\' is already mentioned.", func_decl->name->svalue, 
						table->functions[i]->name->svalue), func_decl->name->context);

	check_entry_func_params(func_decl);
}

uint32_t get_size_of_type(Type* type, Table* table)
{
	if (IS_POINTER_TYPE(type))
		return MACHINE_WORD;
	else if (IS_ENUM_TYPE(type))
		// enum type just ensures that variable can store 
		// 4-byte variable inside
		// so just return the size of i32
		return i32_type.size;
	else if (IS_AGGREGATE_TYPE(type))
		return get_size_of_aggregate_type(type, table);
	else if (IS_PRIMITIVE_TYPE(type))
		return type->size;
	else
		report_error(frmt("Cannot get size of \'%s\' type",
			type_tostr_plain(type)), NULL);
}

uint32_t get_size_of_aggregate_type(Type* type, Table* table)
{
	uint32_t size = 0, buffer = 0;
	uint32_t align = STRUCT_DEFAULT_ALIGNMENT;
	if (IS_STRUCT_TYPE(type))
		for (size_t i = 0; i < sbuffer_len(type->members); i++)
			size += get_size_of_type(type->members[i]->type, table);
	else if (IS_UNION_TYPE(type))
		for (size_t i = 0; i < sbuffer_len(type->members); i++)
			buffer = get_size_of_type(type->members[i]->type, table),
				size = max(size, buffer);
	else if (IS_ARRAY_TYPE(type))
		size = get_size_of_type(type->base, table) *
			evaluate_expr_itype(type->dimension);
	else
		report_error(frmt("Passed type \'%s\' is not aggregate, "
			"in get_user_type_size_in_bytes()"), type_tostr_plain(type), NULL);
	return size;
}

void complete_size(Type* type, Table* table)
{
	switch (type->kind)
	{
	case TYPE_ARRAY:
	case TYPE_POINTER:
		type->size = get_size_of_type(type, table);
		complete_size(type->base, table);
		break;
	case TYPE_UNION:
	case TYPE_STRUCT:
		type->size = get_size_of_aggregate_type(type, table);
		break;
	case TYPE_ENUM:
		type->size = get_size_of_type(type, table);
		break;
	}
}

void complete_type(Type* type, Table* table)
{
	/*
		Completes the missing information in type structure
		if its struct or union type + gets size of it
	*/
	StructDecl* user_type = NULL;
	Type* base = get_base_type(type);
	if (user_type = get_struct(base->repr, table))
		base->kind = TYPE_STRUCT,
			base->members = user_type->members;
	else if (get_enum(base->repr, table))
		base->kind = TYPE_ENUM;
	else if (user_type = get_union(base->repr, table))
		base->kind = TYPE_UNION,
			base->members = user_type->members;
}