#include "visitor.h"

//todo: check type referrings to each other (for freeing)
//todo: create extern without file specifying (to be able to add static libs in compiler args)

struct global_visitor_data {
	FuncDecl* entry;
};

static struct global_visitor_data visitor_data = {
	.entry = NULL
};

Visitor* visitor_new()
{
	Visitor* visitor = new(Visitor, visitor);
	visitor->global = table_new(NULL);
	return visitor;
}

void visitor_data_free()
{
	visitor_data.entry = NULL;
}

void visitor_free(Visitor* visitor)
{
	if (visitor)
	{
		table_free(visitor->global);
		visitor_data_free();
		free(visitor);
	}
}

void visit(AstRoot* ast, Visitor* visitor)
{
	visit_scope(ast->stmts, visitor->global);
	for (size_t i = 0; i < sbuffer_len(ast->stmts); i++)
		visit_stmt(ast->stmts[i], visitor->global);
	if (visitor_data.entry == NULL)
		report_error("Cannot find any entry point.", NULL);
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
		report_error(frmt("Unknown statement kind to visit"
			" in function: %s.", __FUNCTION__), NULL);
	}
}

void visit_type(Type* type, SrcContext* context, Table* table)
{
	// if the incomplete type met, it will be user-defined type:
	//		struct, enum, union, void or unknown type
	if (is_incomplete_type(type))
		// after attempt for completing the type, checking if it is incomplete again
		// report error if it is true
		if (complete_type(type, table), is_incomplete_type(type))
			report_error(frmt("Cannot resolve type: \'%s\'.",
				type->repr), context ? context : type->area->begins);
	
	if (is_array_type(type))
	{
		// checking the index of current array dimension
		visit_expr(type->dimension, table);
		if (!is_const_expr(type->dimension))
			report_error2("Index should be constant expression.",
				get_expr_area(type->dimension));
		// call this function recursievly, because it might be 
		// that there are multi-dimensional array type
		visit_type(type->base, context ? context : type->area->begins, table);
	}

	// resolving size for type
	if (type->kind != TYPE_PRIMITIVE)
		complete_size(type, table);
}

void visit_scope(Stmt** stmts, Table* table)
{
	/*
		Function which visits all statements in specified scope
		before the main visit functions are called.
		Mostly needed for declaring statements, which will
		be order independent for other callers.
	*/

#define _str(val) #val

#define _add_decl(kind, name, c)										\
	if (!add_##kind##2(stmts[i]->##kind##_decl, table))					\
		report_error(frmt("%s \'%s\' is already declared.", _str(kind),	\
			stmts[i]->##kind##_decl->name),								\
				stmts[i]->##kind##_decl->c);							\
	break

#define _add_typedecl(kind)											\
	if (!add_##kind##2(stmts[i]->type_decl->##kind##_decl, table))	\
		report_error(frmt("Type \'%s\' is already declared.",		\
			stmts[i]->type_decl->##kind##_decl->name), NULL);		\
	break

	for (size_t i = 0; i < sbuffer_len(stmts); i++) {
		switch (stmts[i]->kind) {
			case STMT_FUNC_DECL:	_add_decl(func, name->svalue, name->context);
			case STMT_LABEL_DECL:	_add_decl(label, label->svalue, label->context);
			case STMT_TYPE_DECL:
				switch (stmts[i]->type_decl->kind) {
					case TYPE_DECL_ENUM:	_add_typedecl(enum);
					case TYPE_DECL_UNION:	_add_typedecl(union);
					case TYPE_DECL_STRUCT:	_add_typedecl(struct);
					default:
						report_error(frmt("Unknown type declaration kind met "
							"in function: %s.", __FUNCTION__), NULL);
				}
				break;
#undef _str
#undef _add_decl
#undef _add_typedecl
		}
	}
}

void visit_expr(Expr* expr, Table* table)
{
	/*
		Function which determines the appropriate visit function
		for specified expression.
	*/

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
		report_error(frmt("Unknown kind of binary expression met"
			" in function: %s", __FUNCTION__), NULL);
	}
	// after the process of visiting the specified expression
	// type checker comes in. (calling the function which will resolve
	// types in specified expression here)
	get_and_set_expr_type(expr, table);
}

void visit_idnt(Idnt* idnt, Table* table, int is_in_assign)
{
	if (is_enum_member(idnt->svalue, table))
		return visit_idnt_as_enum_member(idnt, table), (void)1;

	// checking if identifier is not a function parameter
	if (!is_function_param_passed(idnt->svalue, table))
	{
		// and check if declared and initialized
		if (!is_variable_declared(idnt->svalue, table))
			report_error(frmt("Variable \'%s\' is not declared.",
				idnt->svalue), idnt->context);

		//todo: comeback here later
		// 
		//if (is_in_assign && !is_variable_initialized(idnt->svalue, table))
		//	add_initialized_variable(idnt->svalue, table);
		// 
		//if (!is_variable_initialized(idnt->svalue, table))
		//	report_error(frmt("Variable \'%s\' is not initialized in current scope.",
		//		idnt->svalue), idnt->context);
	}
}

void visit_idnt_as_enum_member(Idnt* idnt, Table* table)
{
	for (Table* parent = table; parent; parent = parent->nested_in)
		for (size_t i = 0; i < sbuffer_len(parent->enums); i++)
			for (size_t j = 0; j < sbuffer_len(parent->enums[i]->enum_decl->members); j++)
				if (strcmp(idnt->svalue, parent->enums[i]->enum_decl->members[j]->name) == 0)
					return idnt->enum_member_value = parent->enums[i]->enum_decl->members[j]->value,
						idnt->is_enum_member = true, (void)1;
}

void visit_func_call(FuncCall* func_call, Table* table)
{
	// trying to get function declaration from the table
	TableEntity* entity = get_function(
		func_call->name, table);

	if (entity == NULL)
		// print resolve error if there are no any declaration for
		// this function call
		return report_error2(frmt("Cannot resolve function \'%s\'.",
			func_call->name), func_call->area), (void)1;

	// synchronize few fields with function call structure
	FuncDecl* func_decl = entity->function;
	func_call->conv = func_decl->conv;
	func_call->spec = func_decl->spec;

	// capture count of params of function declaration and
	// count of passed arguments to function call
	uint32_t args = sbuffer_len(func_call->args);
	uint32_t params = sbuffer_len(func_decl->params);

	if (args < params)
		report_error2(frmt("More arguments expected for " 
			"function call \'%s\'.", func_call->name), func_call->area);
	else if (args > params && !func_decl->spec->is_vararg)
		// print this error only when the function declaration has vararg property
		report_error2(frmt("Less arguments expected for "
			"function call \'%s\'.", func_call->name), func_call->area);

	// resolvig all passed arguments to function call
	for (size_t i = 0; i < sbuffer_len(func_call->args); i++)
		visit_expr(func_call->args[i], table);
}

void visit_unary_expr(UnaryExpr* unary_expr, Table* table)
{
	if (unary_expr->expr)
		visit_expr(unary_expr->expr, table);
	switch (unary_expr->kind)
	{
	case UNARY_CAST:
	case UNARY_SIZEOF:
		// these unary expression does not have their own internal expression
		// just check the type of each (existence mostly)
		visit_type(unary_expr->cast_type,
			unary_expr->area->begins, table);
		break;
	case UNARY_ADDRESS:
	case UNARY_DEREFERENCE:
		// check for addressable value
		if (!is_addressable_value(unary_expr->expr, table))
			report_error2("Addressable expression required.", 
				unary_expr->area);
		break;
	case UNARY_PLUS:
	case UNARY_MINUS:
	case UNARY_LG_NOT:
	case UNARY_BW_NOT:
	case UNARY_LENGTHOF:
		// nothing to do here ...
		break;
	default:
		report_error("Unknown kind of unary expression met"
			" in visit_unary_expr()", NULL);
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
		//visit_array_member_accessor(binary_expr, table);
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
			report_error2("Cannot assign something to non-addressable value.",
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
			report_error2("Cannot assign something to non-addressable value.", 
				get_expr_area(binary_expr->lexpr));
		visit_expr(binary_expr->lexpr, table);
		visit_expr(binary_expr->rexpr, table);
		break;
	default:
		report_error(frmt("Unknown kind of binary expression met"
			" in function: %s", __FUNCTION__), NULL);
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
	Type* type = retrieve_expr_type(condition);
	if (!is_numeric_type(type) && !is_pointer_like_type(type))
		report_error2(frmt("Condition expression cannot have \'%s\' type.",
			type_tostr_plain(type)), get_expr_area(condition));
}

void visit_if_stmt(IfStmt* if_stmt, Table* table)
{
	Table* local = table_new(table);
	visit_condition(if_stmt->cond, table);
	visit_scope(if_stmt->body->stmts, local);
	visit_block(if_stmt->body, local);

	for (size_t i = 0; i < sbuffer_len(if_stmt->elifs); i++)
		visit_elif_stmt(if_stmt->elifs[i], table);

	if (if_stmt->else_body)
		visit_scope(if_stmt->else_body->stmts, local = table_new(table)),
			visit_block(if_stmt->else_body, local);
}

void visit_elif_stmt(ElseIf* elif_stmt, Table* table)
{
	Table* local = table_new(table);
	visit_condition(elif_stmt->cond, table);
	visit_scope(elif_stmt->body->stmts, local);
	visit_block(elif_stmt->body, local);
}

void resolve_conjuction_collision(SwitchStmt* switch_stmt)
{
	//todo: test this
	size_t cases = sbuffer_len(switch_stmt->cases);
	if (cases > 0 && switch_stmt->cases[cases - 1]->is_conjucted)
		report_error2("Case's body expected here.",
			get_expr_area(switch_stmt->cases[cases - 1]->value));
}

void resolve_duplicated_conditions(SwitchStmt* switch_stmt, Table* table)
{
	Expr** conditions = NULL;

	for (size_t i = 0; i < sbuffer_len(switch_stmt->cases); i++)
	{
		Expr* case_value = switch_stmt->cases[i]->value;
		switch (case_value->kind)
		{
		case EXPR_IDNT:
			if (!case_value->idnt->is_enum_member)
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

	resolve_conjuction_collision(switch_stmt);
	resolve_duplicated_conditions(switch_stmt, table);

	for (size_t i = 0; i < sbuffer_len(switch_stmt->cases); i++)
	{
		local = table_new(table);
		local->in_switch = switch_stmt;

		if (switch_stmt->cases[i]->body)
			visit_scope(switch_stmt->cases[i]->body->stmts, local),
				visit_block(switch_stmt->cases[i]->body, local);

		switch_case_type = get_expr_type(switch_stmt->cases[i]->value, local);
		if (!can_cast_implicitly(switch_cond_type, switch_case_type))
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
		report_error(frmt("Unknown loop kind met",
			" in function: %s", __FUNCTION__), NULL);
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
		report_error(frmt("Unknown jump statement kind met", 
			" in function: %s", __FUNCTION__), NULL);
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
	Type* type = !var_decl->is_auto ? var_decl->type_var->type : 
		get_expr_type(var_decl->var_init, table);
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
		visit_type(type, type->area ? type->area->begins : NULL, table);
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
			if (var_decl->is_auto)
				var_decl->type_var->type = init_expr_type;
		}
	}
}

void visit_enum(EnumDecl* enum_decl, Table* table)
{
	/*
		Resolving semantically enum declaration.
			- duplicate check in current enum.
			- duplicate check in other enums scoped.
			- constant expression check.
			- expression's type check.
	*/

	for (size_t i = 0; i < sbuffer_len(enum_decl->members); i++) {
		EnumMember* current = enum_decl->members[i];

		// Validation of assigned expression, it must be constant.
		if (current->value && !is_const_expr(current->value))
			report_error2(frmt("Enum member \'%s\' must have constant expression, in \'%s\'.",
				current->name, enum_decl->name), get_expr_area(current->value));
		
		// Visiting expression assigned to enum member
		if (current->value) {
			visit_expr(current->value, table);
			// Retrieving assigned type for validation
			Type* expr_type = retrieve_expr_type(current->value);
			if (expr_type != NULL) {
				// Type must be integral and less or equal than i32/u32.
				if (!is_integral_type(expr_type) || expr_type->size > i32_type.size) {
					report_error2(frmt("Enum's member \'%s\' has incompatible \'%s\' type.",
						current->name, type_tostr_plain(expr_type)), get_expr_area(current->value));
				}
			}
		}

		// Trying to add enum member to symbol table,
		// if this enum member already declared somewhere in this scope
		// this function will return false, and error will be printed
		if (!add_enum_member(current, table)) {
			TableEntity* entity = NULL;
			if (!(entity = get_enum_member(current->name, table))) {
			 	// this is ambigiuos situation when add_enum_member finds entity,
				// but get_enum_member cannot find any.
				report_error(frmt("Retrieved NULL table entity, this is a bug actually:"
					"line %d, function %s.", __LINE__, __FUNCTION__), NULL);
			}
			else {
				report_error(frmt("Enum member \'%s\' in \'%s\' is already declared in \'%s\'.",
					current->name, enum_decl->name, entity->enum_member->from->name), current->context);
			}
		}
	}
}

void visit_union(UnionDecl* union_decl, Table* table)
{
	/*
		Resolving semantically union declaration.
			- duplicate check in current union.
			- resolving member's type.
	*/

	visit_members(union_decl->name, 
		union_decl->members, table);
}

void visit_struct(StructDecl* struct_decl, Table* table)
{
	/*
		Resolving semantically struct declaration.
			- duplicate check in current struct.
			- resolving member's type.
	*/

	visit_members(struct_decl->name,
		struct_decl->members, table);
}

void visit_members(const char* type, Member** members, Table* table)
{
	/*
		Resolves semantically set of members, usually passed by 
		union/struct declaration.
	*/

	for (size_t i = 0; i < sbuffer_len(members); i++)
	{
		visit_type(members[i]->type,
			members[i]->area->begins, table);
		if (strcmp(members[i]->type->repr, type) == 0)
			if (members[i]->type->kind != TYPE_POINTER)
				report_error2(frmt("Member declared with unresolved type \'%s\'.",
					type), members[i]->area);
		for (size_t j = 0; j < sbuffer_len(members); j++)
		{
			if (members[i] == members[j])
				continue;
			if (strcmp(members[i]->name, members[j]->name) == 0)
				report_error2(frmt("Member is already declared in \'%s\'",
					type), members[i]->area);
		}
	}
}

void visit_type_decl_stmt(TypeDecl* type_decl, Table* table)
{
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
		report_error(frmt("Unknown kind of type declaration met"
			" in function: %s.", __FUNCTION__), NULL);
	}
}

void visit_block(Block* block, Table* table)
{
	for (size_t i = 0; i < sbuffer_len(block->stmts); i++)
		visit_stmt(block->stmts[i], table);
}

void visit_func_decl_callconv(FuncDecl* func_decl)
{
	if (func_decl->conv->kind == CALL_CONV_STDCALL &&
			func_decl->spec->is_vararg)
		report_error("Cannot use __VA_ARGS__ with stdcall.",
			func_decl->name->context);
}

void visit_func_decl_specs(FuncDecl* func_decl, Table* table)
{
	if (func_decl->spec->is_entry)
		visit_entry_func_stmt(func_decl, table);
	
	visit_func_decl_callconv(func_decl);

	// check the possibility of existence of function's body in case 
	// when function is external or not.
	if (func_decl->spec->is_external && func_decl->body)
		report_error(frmt("Function \'%s\' specified like external.",
			func_decl->name->svalue), func_decl->name->context);
	if (!func_decl->spec->is_external && !func_decl->body)
		report_error(frmt("Function \'%s\' needs body, its not external.",
			func_decl->name->svalue), func_decl->name->context);
}

void visit_func_decl_stmt(FuncDecl* func_decl, Table* table)
{
	// case when function is declared inside another function or other local scope
	// probably this error should be resolved syntactically (but i'm not sure about this)
	if (table->nested_in)
		report_error("Cannot declare function not in global scope",
			func_decl->name->context);
	
	Table* local = table_new(table);
	local->in_function = func_decl;

	visit_func_decl_specs(func_decl, table);
	// do not visit scope statements if the function is external (there are no any body in that case)
	if (!func_decl->spec->is_external)
		visit_scope(func_decl->body->stmts, local);

	for (size_t i = 0; i < sbuffer_len(func_decl->params); i++)
	{
		// checking if the parameter's name is enum member or
		// this name is already passed as function parameter
		if (is_function_param_passed(func_decl->params[i]->var, local) || 
				is_enum_member(func_decl->params[i]->var, local))
			report_error2(frmt("Parameter's name \'%s\' is in use already.",
				func_decl->params[i]->var), func_decl->params[i]->area);
		visit_type(func_decl->params[i]->type, 
			func_decl->params[i]->area->begins, table);
	}

	// no need for type resolving in case of void
	// because it just means that function does not return any value
	// (void in this context just exceptional case)
	if (func_decl->type->kind != TYPE_VOID)
		visit_type(func_decl->type, 
			func_decl->name->context, table);

	// no need to check body if this function is external
	// and also no need for return flow check
	if (!func_decl->spec->is_external)
		visit_block(func_decl->body, local),
			check_func_return_flow(func_decl);
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
	if (visitor_data.entry != NULL)
		report_error("Entry function is already declared here.", 
			visitor_data.entry->name->context);
	visitor_data.entry = func_decl;
	check_entry_func_params(func_decl);
}

uint32_t get_size_of_type(Type* type, Table* table)
{
	switch (type->kind)
	{
	case TYPE_ENUM:
		// enum type just ensures that variable can store 
		// 4-byte variable inside
		// so just return the size of i32
		return i32_type.size;
	case TYPE_POINTER:
		return MACHINE_WORD;
	case TYPE_PRIMITIVE:
		return type->size;
	default:
		if (is_aggregate_type(type))
			return get_size_of_aggregate_type(type, table);
		report_error(frmt("Cannot get size of \'%s\' type",
			type_tostr_plain(type)), NULL);
	}
	return 0;
}

uint32_t get_size_of_aggregate_type(Type* type, Table* table)
{
	uint32_t size = 0, buffer = 0;
	
	switch (type->kind)
	{
	case TYPE_UNION:
	case TYPE_STRUCT:
		for (size_t i = 0; i < sbuffer_len(type->members); i++) 
		{
			buffer = type->members[i]->type->size;
			// when type is union, calculate max between size and size of current member
			// otherwise just add member size to whole size (in case of struct)
			size = type->kind == TYPE_UNION ? 
				max(size, buffer) : (size + buffer);
		}
		return size;
	case TYPE_ARRAY:
		return (uint32_t)(get_size_of_type(type->base, table) *
			evaluate_expr_itype(type->dimension));
	default:
		report_error(frmt("Passed type \'%s\' is not aggregate, "
			"in get_size_of_aggregate_type()", type_tostr_plain(type)), NULL);
	}
	return 0;
}

void complete_size(Type* type, Table* table)
{
	// variable in which whole 
	// member's offset will be stored
	uint32_t offset = 0;
	switch (type->kind)
	{
	case TYPE_ENUM:
		type->size = get_size_of_type(type, table);
		break;
	case TYPE_ARRAY:
	case TYPE_POINTER:
		type->size = get_size_of_type(type, table);
		complete_size(type->base, table);
		break;
	case TYPE_UNION:
	case TYPE_STRUCT:
		// completing offsets for members (struct's or union's)
		for (uint32_t i = 0; i < sbuffer_len(type->members); i++)
		{
			type->members[i]->type->size = get_size_of_type(
				type->members[i]->type, table);
			// in case of union, dont add offset for member, 
			// keep it 0 for all members (here there are no logic for that
			// because Member struct is allocated already with 0 stored in offset)
			if (type->kind == TYPE_STRUCT)
				type->members[i]->offset = offset,
					offset += type->members[i]->type->size;
		}
		type->size = get_size_of_aggregate_type(type, table);
		break;
	}
}

void complete_type(Type* type, Table* table)
{
	/*
		Completes the missing information in type structure
		if its struct, enum or union type + gets size of it
	*/
	TableEntity* user_type = NULL;
	Type* base = get_base_type(type);

	// type void cannot be completed
	if (type->kind == TYPE_VOID)
		return;
	if (user_type = get_struct(base->repr, table))
		base->kind = TYPE_STRUCT,
			base->members = user_type->struct_decl->members;
	else if (get_enum(base->repr, table))
		base->kind = TYPE_ENUM;
	else if (user_type = get_union(base->repr, table))
		base->kind = TYPE_UNION,
			base->members = user_type->union_decl->members;
}

bool is_const_expr(Expr* expr)
{
	if (!expr)
		return false;
	switch (expr->kind)
	{
	case EXPR_CONST:
		return true;
	case EXPR_IDNT:
		return expr->idnt->is_enum_member;
	case EXPR_UNARY_EXPR:
		switch (expr->unary_expr->kind)
		{
		case UNARY_SIZEOF:
		case UNARY_LENGTHOF:
			return true;
		}
		return is_const_expr(expr->unary_expr->expr);
	case EXPR_BINARY_EXPR:
		return is_const_expr(expr->binary_expr->lexpr) &&
			is_const_expr(expr->binary_expr->rexpr);
	case EXPR_TERNARY_EXPR:
		return is_const_expr(expr->ternary_expr->lexpr) &&
			is_const_expr(expr->ternary_expr->rexpr) &&
			is_const_expr(expr->ternary_expr->cond);
	}
	return false;
}

bool is_primary_expr(Expr* expr)
{
	switch (expr->kind)
	{
	case EXPR_IDNT:
	case EXPR_CONST:
	case EXPR_STRING:
		return true;
	}
	return false;
}

bool is_enum_member(const char* var, Table* table)
{
	for (Table* parent = table; parent; parent = parent->nested_in)
		for (size_t i = 0; i < sbuffer_len(parent->enums); i++)
			for (size_t j = 0; j < sbuffer_len(parent->enums[i]->enum_decl->members); j++)
				if (strcmp(var, parent->enums[i]->enum_decl->members[j]->name) == 0)
					return true;
	return false;
}

bool is_addressable_value(Expr* expr)
{
	if (!expr)
		return false;
	switch (expr->kind)
	{
	case EXPR_CONST:
	case EXPR_STRING:
	case EXPR_INITIALIZER:
	case EXPR_TERNARY_EXPR:
		return false;

	case EXPR_IDNT:
		// in case of idnt we need to be sure that
		// the idnt is variable, not enum member
		return !expr->idnt->is_enum_member;
	case EXPR_UNARY_EXPR:
		switch (expr->unary_expr->kind)
		{
		case UNARY_DEREFERENCE:
			return true;
		default:
			return false;
		}
		break;
	case EXPR_BINARY_EXPR:
		switch (expr->binary_expr->kind)
		{
		case BINARY_ASSIGN:
		case BINARY_ADD_ASSIGN:
		case BINARY_SUB_ASSIGN:
		case BINARY_MUL_ASSIGN:
		case BINARY_DIV_ASSIGN:
		case BINARY_MOD_ASSIGN:
		case BINARY_LSHIFT_ASSIGN:
		case BINARY_RSHIFT_ASSIGN:

		case BINARY_BW_OR_ASSIGN:
		case BINARY_BW_AND_ASSIGN:
		case BINARY_BW_XOR_ASSIGN:

		case BINARY_MEMBER_ACCESSOR:
		case BINARY_PTR_MEMBER_ACCESSOR:
		case BINARY_ARR_MEMBER_ACCESSOR:
			return true;
		default:
			return false;
		}
		break;
	default:
		return false;
	}
	return false;
}