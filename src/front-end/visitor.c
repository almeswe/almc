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
	Visitor* visitor = new(Visitor);
	visitor->global = table_new(NULL);
	return visitor;
}

void visitor_data_free()
{
	visitor_data.entry = NULL;
}

void visitor_free(Visitor* visitor)
{
	if (visitor) {
		table_free(visitor->global);
		visitor_data_free();
		free(visitor);
	}
}

void visit(AstRoot* ast, Visitor* visitor)
{
	visit_scope(ast->stmts, visitor->global);
	for (size_t i = 0; i < sbuffer_len(ast->stmts); i++) {
		visit_stmt(ast->stmts[i], visitor->global);
	}
	if (visitor_data.entry == NULL) {
		report_error("Cannot find any entry point.", NULL);
	}
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
		visit_block(stmt->block, table);
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
	case STMT_EMPTY:
	case STMT_LABEL_DECL:
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
	if (is_incomplete_type(type)) {
		// after attempt for completing the type, checking if it is incomplete again
		// report error if it is true
		if (complete_type(type, table), is_incomplete_type(type)) {
			report_error(frmt("Cannot resolve type: \'%s\'.",
				type->repr), context ? context : type->area->begins);
		}
	}

	if (is_array_type(type)) {
		// checking the index of current array dimension
		visit_expr(type->attrs.arr.dimension, table);
		if (!is_const_expr(type->attrs.arr.dimension)) {
			report_error2("Index should be constant expression.",
				get_expr_area(type->attrs.arr.dimension));
		}
		// call this function recursievly, because it might be 
		// that there are multi-dimensional array type
		visit_type(type->base, context ? context : type->area->begins, table);
	}

	// resolving size for type
	if (type->kind != TYPE_PRIMITIVE) {
		complete_size(type, table);
	}
}

void visit_scope(Stmt** stmts, Table* table)
{
	/*
		Function which visits all statements in specified scope
		before the main visit functions are called.
		Mostly needed for declaring statements, which will
		be order independent for other callers.
	*/

	FuncDecl** func_decls = NULL;

#define _str(val) #val

#define _add_decl(kind, name, c)										\
	if (!add_##kind##(stmts[i]->##kind##_decl, table))					\
		report_error(frmt("%s \'%s\' is already declared.", _str(kind),	\
			stmts[i]->##kind##_decl->name),								\
				stmts[i]->##kind##_decl->c)							

#define _add_typedecl(kind, c)										\
	if (!add_##kind##(stmts[i]->type_decl->##kind##_decl, table))	\
		report_error(frmt("Type \'%s\' is already declared.",		\
			stmts[i]->type_decl->##kind##_decl->name->value),		\
			stmts[i]->type_decl->##kind##_decl->c)					

	for (size_t i = 0; i < sbuffer_len(stmts); i++) {
		switch (stmts[i]->kind) {
			case STMT_FUNC_DECL:
				_add_decl(func, name->value, name->context);
				sbuffer_add(func_decls, stmts[i]->func_decl);
				break;
			case STMT_LABEL_DECL:	
				_add_decl(label, name->value, name->context);
				break;
			case STMT_TYPE_DECL:
				switch (stmts[i]->type_decl->kind) {
					case TYPE_DECL_ENUM:	
						_add_typedecl(enum, name->context);
						visit_enum(stmts[i]->type_decl->enum_decl, table);
						break;
					case TYPE_DECL_UNION:	
						_add_typedecl(union, name->context);
						break;
					case TYPE_DECL_STRUCT:	
						_add_typedecl(struct, name->context);
						break;
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
	
	/*
		Resolving types of parameters of each function
		and its return type, to avoid incomplete types during
		function calls before the actual function declaration 
		resolving process is done.
	*/

	for (size_t i = 0; i < sbuffer_len(func_decls); i++) {
		for (size_t p = 0; p < sbuffer_len(func_decls[i]->params); p++) {
			visit_type(func_decls[i]->params[p]->type, NULL, table);
		}
		// no need for type resolving in case of void
		// because it just means that function does not return any value
		// (void in this context just exceptional case)
		if (func_decls[i]->type->kind != TYPE_VOID) {
			visit_type(func_decls[i]->type, NULL, table);
		}
	}
	sbuffer_free(func_decls);
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

void visit_addr_expr(Expr* expr, Table* table)
{
	visit_expr(expr, table);
	if (!is_addressable_expr(expr)) {
		report_error2("Expression must be addressable.", 
			get_expr_area(expr));
	}
}

void visit_idnt(Idnt* idnt, Table* table, bool is_in_assign)
{
	TableEntity* entity = NULL;
	if (entity = get_enum_member(idnt->svalue, table)) {
		idnt->attrs.is_enum_member = true;
		idnt->attrs.enum_member = entity->value.enum_member;
	}
	else {
		if (!get_parameter(idnt->svalue, table)) {
			if (!(entity = get_variable(idnt->svalue, table))) {
				report_error(frmt("Unknown entity name \'%s\'.",
					idnt->svalue), idnt->context);
			}
		}
		//todo: comeback here later
		//	//if (is_in_assign && !is_variable_initialized(idnt->svalue, table))
		//	//	add_initialized_variable(idnt->svalue, table);
		//	// 
		//	//if (!is_variable_initialized(idnt->svalue, table))
		//	//	report_error(frmt("Variable \'%s\' is not initialized in current scope.",
		//	//		idnt->svalue), idnt->context);
		//}
	}
}

void visit_func_call(FuncCall* func_call, Table* table)
{
	// trying to get function declaration from the table
	TableEntity* entity = get_function(
		func_call->name->value, table);

	if (entity == NULL) {
		// print resolve error if there are no any declaration for
		// this function call
		return report_error2(frmt("Cannot resolve function \'%s\'.",
			func_call->name->value), func_call->area), (void)1;
	}

	// synchronize few fields with function call structure
	FuncDecl* func_decl = entity->value.function;
	func_call->decl = func_decl;

	// capture count of params of function declaration and
	// count of passed arguments to function call
	uint32_t args = sbuffer_len(func_call->args);
	uint32_t params = sbuffer_len(func_decl->params);

	if (args < params) {
		report_error(frmt("More arguments expected for function call \'%s\'.",
			func_call->name->value), func_call->name->context);
	}
	else if (args > params && !func_decl->spec->is_vararg) {
		// print this error only when the function declaration has vararg property
		report_error(frmt("Less arguments expected for function call \'%s\'.",
			func_call->name->value), func_call->name->context);
	}

	// resolvig all passed arguments to function call
	for (size_t i = 0; i < sbuffer_len(func_call->args); i++) {
		visit_expr(func_call->args[i], table);
	}
}

void visit_unary_expr(UnaryExpr* unary_expr, Table* table)
{
	if (unary_expr->expr) {
		visit_expr(unary_expr->expr, table);
	}
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
		if (!is_addressable_expr(unary_expr->expr)) {
			report_error2("Addressable expression required.",
				unary_expr->area);
		}
		break;
	case UNARY_PLUS:
	case UNARY_MINUS:
	case UNARY_LG_NOT:
	case UNARY_BW_NOT:
	case UNARY_LENGTHOF:
		// nothing to do here ...
		break;
	default:
		report_error(frmt("Unknown kind of unary expression met"
			" in function: ", __FUNCTION__), NULL);
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
		visit_expr(binary_expr->lexpr, table);
		visit_type(retrieve_expr_type(binary_expr->lexpr), 
			binary_expr->area->begins, table);
		break;
	//case BINARY_ASSIGN:
		// if right expression is idnt, then set it as initialized
		//if (binary_expr->lexpr->kind == EXPR_IDNT) {
		//	visit_idnt(binary_expr->lexpr->idnt, table, 1);
		//}
		//else {
		//	visit_expr(binary_expr->lexpr, table);
		//}
		//visit_addr_expr(binary_expr->lexpr, table);
		//visit_expr(binary_expr->rexpr, table);
		//break;
	case BINARY_ASSIGN:
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
		visit_addr_expr(binary_expr->lexpr, table);
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
	if (!is_numeric_type(type) && !is_pointer_like_type(type)) {
		report_error2(frmt("Condition expression cannot have \'%s\' type.",
			type_tostr_plain(type)), get_expr_area(condition));
	}
}

void visit_if_stmt(IfStmt* if_stmt, Table* table)
{
	Table* local = table_new(table);
	visit_condition(if_stmt->cond, table);
	visit_scope(if_stmt->body->stmts, local);
	visit_block_stmts(if_stmt->body, local);

	for (size_t i = 0; i < sbuffer_len(if_stmt->elifs); i++) {
		visit_elif_stmt(if_stmt->elifs[i], table);
	}

	if (if_stmt->else_body) {
		visit_scope(if_stmt->else_body->stmts, local = table_new(table)),
			visit_block_stmts(if_stmt->else_body, local);
	}
}

void visit_elif_stmt(ElseIf* elif_stmt, Table* table)
{
	Table* local = table_new(table);
	visit_condition(elif_stmt->cond, table);
	visit_scope(elif_stmt->body->stmts, local);
	visit_block_stmts(elif_stmt->body, local);
}

void visit_switch_stmt(SwitchStmt* switch_stmt, Table* table)
{
	visit_condition(switch_stmt->cond, table);

	Type* switch_case_type = NULL;
	Type* switch_cond_type = get_expr_type(switch_stmt->cond, table);
	SrcArea* switch_case_area = NULL;

	size_t cases = sbuffer_len(switch_stmt->cases);
	if (cases > 0 && switch_stmt->cases[cases - 1]->is_conjucted) {
		report_error2("Expected body here.",
			get_expr_area(switch_stmt->cases[cases - 1]->value));
	}

	for (size_t i = 0; i < cases; i++) {
		Table* local = table_new(table);
		local->scope_refs.in_switch = switch_stmt;

		if (switch_stmt->cases[i]->body) {
			visit_scope(switch_stmt->cases[i]->body->stmts, local);
			visit_block_stmts(switch_stmt->cases[i]->body, local);
		}

		switch_case_area = get_expr_area(switch_stmt->cases[i]->value);
		switch_case_type = get_expr_type(switch_stmt->cases[i]->value, local);
		if (!cast_implicitly(switch_cond_type, switch_case_type, switch_case_area)) {
			report_error2("Unexpected NULL type value returned.", switch_case_area);
		}
		if (!is_integral_type(switch_case_type)) {
			report_error2(frmt("Condition of case statement cannot be \'%s\'",
				type_tostr_plain(switch_case_type)), switch_case_area);
		}
	}

	if (switch_stmt->default_case) {
		Table* local = table_new(table);
		local->scope_refs.in_switch = switch_stmt;
		visit_scope(switch_stmt->default_case->stmts, local);
		visit_block_stmts(switch_stmt->default_case, local);
	}
}

void visit_loop_stmt(LoopStmt* loop_stmt, Table* table)
{
	Table* local = table_new(table);
	local->scope_refs.in_loop = loop_stmt;
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
	visit_block_stmts(do_loop->body, table);
	visit_scope(do_loop->body->stmts, table);
	visit_condition(do_loop->cond, table);
}

void visit_for_loop_stmt(ForLoop* for_loop, Table* table)
{
	if (for_loop->init) {
		visit_var_decl_stmt(for_loop->init, table);
	}
	if (for_loop->cond) {
		visit_condition(for_loop->cond, table);
	}
	if (for_loop->step) {
		visit_expr(for_loop->step, table);
	}
	visit_scope(for_loop->body->stmts, table);
	visit_block_stmts(for_loop->body, table);
}

void visit_while_loop_stmt(WhileLoop* while_loop, Table* table)
{
	visit_condition(while_loop->cond, table);
	visit_scope(while_loop->body->stmts, table);
	visit_block_stmts(while_loop->body, table);
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
	if (!goto_stmt->expr) {
		report_error2(frmt("Expression in jump statement is NULL."
			" in function: %s.", __FUNCTION__), goto_stmt->area);
	}
	else {
		Expr* label = goto_stmt->expr;
		if (label->kind != EXPR_IDNT) {
			report_error2("Expression in jump statement must be identifier"
				" in case of goto statement.", goto_stmt->area);
		}
		if (!get_label(label->idnt->svalue, table)) {
			report_error2(frmt("Label \'%s\' is not declared in current scope.",
				label->idnt->svalue), goto_stmt->area);
		}
	}
}

void visit_break_stmt(JumpStmt* break_stmt, Table* table)
{
	if (!table->scope_refs.in_loop && !table->scope_refs.in_switch) {
		report_error2("Cannot use break statement"
			" in this context.", break_stmt->area);
	}
}

void visit_return_stmt(JumpStmt* return_stmt, Table* table)
{
	/*
		Function which resolves returns statement.
			- in-function validation.
			- return expression type.
			- function return type.
	*/

	if (!table->scope_refs.in_function) {
		report_error2("Cannot use return statement"
			" when its not located in function.", return_stmt->area);
	}
	else {
		Expr* ret_expr = return_stmt->expr;
		Type* func_ret_type = table->scope_refs.in_function->type;
		if (ret_expr != NULL) {
			visit_expr(ret_expr, table);
			Type* ret_expr_type = retrieve_expr_type(ret_expr);
			if (!can_cast_implicitly(func_ret_type, ret_expr_type)) {
				report_error2(frmt("Cannot return value of type \'%s\' from function with \'%s\'.",
					type_tostr_plain(ret_expr_type), type_tostr_plain(func_ret_type)),
						get_expr_area(ret_expr));
			}
		}
		else {
			if (func_ret_type->kind != TYPE_VOID) {
				report_error2("Cannot ignore return expression"
					" not in void function.", return_stmt->area);
			}
		}
	}
}

void visit_continue_stmt(JumpStmt* continue_stmt, Table* table)
{
	if (!table->scope_refs.in_loop) {
		report_error2("Cannot use continue statement"
			" in this context.", continue_stmt->area);
	}
}

void visit_var_decl_stmt(VarDecl* var_decl, Table* table)
{
	if (!add_variable(var_decl, table)) {
		TableEntity* entity = NULL;
		SrcArea* area = var_decl->type_var->area;
		const char* name = var_decl->type_var->var;
		// if get_variable returns NULL, it means that there are
		// no variable with specified name, and other entity named like this variable
		if (!(entity = get_variable(name, table))) {
			report_error2(frmt("Variable's name \'%s\' is already"
				" in use by another entity.", name), area);
		}
		else {
			report_error2(frmt("Variable \'%s\'"
				" is already declared.", name), area);
		}
	}

	// if variable declaration is auto,
	// it means that we need to specify type manually
	if (var_decl->var_init != NULL) {
		visit_expr(var_decl->var_init, table);
	}
	if (var_decl->is_auto) {
		if (var_decl->var_init == NULL) {
			report_error2("Auto variable declaration statement "
				"requires expression initializer.", var_decl->type_var->area);
		}
		var_decl->type_var->type = retrieve_expr_type(var_decl->var_init);
	}
	else {
		// in case of common variable declaration
		// we need to ensure that type by the right side is subset of left type.
		Type* ltype = var_decl->type_var->type;
		visit_type(ltype, var_decl->type_var->area->begins, table);
		if (var_decl->var_init != NULL) {
			Type* rtype = retrieve_expr_type(var_decl->var_init);
			if (!can_cast_implicitly(ltype, rtype)) {
				report_error2(frmt("Expression-initializer has incompatible type \'%s\' with type of variable \'%s\'.",
					type_tostr_plain(rtype), type_tostr_plain(ltype)), get_expr_area(var_decl->var_init));
			}
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

		// Validation of assigned expression, it must be constant.
		if (current->value && !is_const_expr(current->value)) {
			report_error2(frmt("Enum member \'%s\' must have constant expression, in \'%s\'.",
				current->name, enum_decl->name->value), get_expr_area(current->value));
		}

		// Trying to add enum member to symbol table,
		// if this enum member already declared somewhere in this scope
		// this function will return false, and error will be printed
		if (!add_enum_member(current, table)) {
			TableEntity* entity = NULL;
			if (!(entity = get_enum_member(current->name, table))) {
			 	// this is ambigiuos situation when add_enum_member finds entity,
				// but get_enum_member cannot find any.
				report_error(frmt("Enum member's name \'%s\' is already"
					" in use by another entity.", current->name), current->context);
			}
			else {
				report_error(frmt("Enum member \'%s\' in \'%s\' is already declared in \'%s\'.",
					current->name, enum_decl->name->value, entity->value.enum_member->from->name->value), current->context);
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

	visit_members(union_decl->name->value,
		union_decl->members, table);
}

void visit_struct(StructDecl* struct_decl, Table* table)
{
	/*
		Resolving semantically struct declaration.
			- duplicate check in current struct.
			- resolving member's type.
	*/

	visit_members(struct_decl->name->value,
		struct_decl->members, table);
}

void visit_members(const char* type, Member** members, Table* table)
{
	/*
		Resolves semantically set of members, usually passed by 
		union/struct declaration.
	*/

	for (size_t i = 0; i < sbuffer_len(members); i++) {
		visit_type(members[i]->type,
			members[i]->area->begins, table);
		if (strcmp(members[i]->type->repr, type) == 0) {
			if (members[i]->type->kind != TYPE_POINTER) {
				report_error2(frmt("Member declared with unresolved type \'%s\'.",
					type), members[i]->area);
			}
		}
		for (size_t j = 0; j < sbuffer_len(members); j++) {
			if (members[i] == members[j]) {
				continue;
			}
			if (strcmp(members[i]->name, members[j]->name) == 0) {
				report_error2(frmt("Member is already declared in \'%s\'",
					type), members[i]->area);
			}
		}
	}
}

void visit_type_decl_stmt(TypeDecl* type_decl, Table* table)
{
	switch (type_decl->kind)
	{
		case TYPE_DECL_ENUM:
			// enum is already visited in visit_scope function
			// it is needed for pre-definition of all enum members in symbol table
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
	/*
		Function which visits all statements and scope.
		And also creates new local scope.
	*/

	Table* local = table_new(table);
	visit_scope(block->stmts, local);
	visit_block_stmts(block, local);
}

void visit_block_stmts(Block* block, Table* table)
{
	/*
		Function which just visits all statements in scope.
	*/

	for (size_t i = 0; i < sbuffer_len(block->stmts); i++) {
		visit_stmt(block->stmts[i], table);
	}
}

void visit_func_decl_callconv(FuncDecl* func_decl)
{
	if (func_decl->conv->kind == CALL_CONV_STDCALL &&
			func_decl->spec->is_vararg) {
		report_error("Cannot use __VA_ARGS__ with stdcall.",
			func_decl->name->context);
	}
}

void visit_func_decl_specs(FuncDecl* func_decl, Table* table)
{
	if (func_decl->spec->is_entry) {
		visit_entry_func_stmt(func_decl, table);
	}

	visit_func_decl_callconv(func_decl);

	// check the possibility of existence of function's body in case 
	// when function is external or not.
	if (func_decl->spec->is_external && func_decl->body) {
		report_error(frmt("Function \'%s\' specified like external.",
			func_decl->name->value), func_decl->name->context);
	}
	if (!func_decl->spec->is_external && !func_decl->body) {
		report_error(frmt("Function \'%s\' needs body, its not external.",
			func_decl->name->value), func_decl->name->context);
	}
}

void visit_func_decl_stmt(FuncDecl* func_decl, Table* table)
{
	// case when function is declared inside another function or other local scope
	// probably this error should be resolved syntactically (but i'm not sure about this)
	if (table->nested_in) {
		report_error("Cannot declare function not in global scope",
			func_decl->name->context);
	}
	
	Table* local = table_new(table);
	local->scope_refs.in_function = func_decl;

	visit_func_decl_specs(func_decl, table);
	// do not visit scope statements if the function is external 
	// (there are no any body in that case)
	if (!func_decl->spec->is_external) {
		visit_scope(func_decl->body->stmts, local);
	}

	for (size_t i = 0; i < sbuffer_len(func_decl->params); i++) {
		TypeVar* current = func_decl->params[i];
		// checking if the parameter's name is enum member or
		// this name is already passed as function parameter
		if (!add_parameter(current, table)) {
			TableEntity* entity = NULL;
			// if get_parameter returns NULL, it means that there are
			// no parameter with specified name, and other entity named like this parameter
			if (!(entity = get_parameter(current->var, table))) {
				report_error2(frmt("Parameter's name \'%s\' is already"
					" in use by another entity.", current->var), current->area);
			}
			else {
				report_error2(frmt("Parameter with name \'%s\'"
					" is already passed.", current->var), current->area);
			}
		}
	}

	// no need to check body if this function is external
	// and also no need for return flow check
	if (!func_decl->spec->is_external) {
		visit_block_stmts(func_decl->body, local),
			check_func_return_flow(func_decl);
	}
}

void visit_import_stmt(ImportStmt* import_stmt, Table* table)
{
	visit_scope(import_stmt->ast->stmts, table);
	for (size_t i = 0; i < sbuffer_len(import_stmt->ast->stmts); i++) {
		visit_stmt(import_stmt->ast->stmts[i], table);
	}
}

void visit_entry_func_stmt(FuncDecl* func_decl, Table* table)
{
	if (visitor_data.entry != NULL) {
		report_error("Entry function is already declared here.",
			visitor_data.entry->name->context);
	}
	visitor_data.entry = func_decl;

	size_t param_count = sbuffer_len(func_decl->params);
	if (param_count != 2) {
		if (param_count != 0) {
			report_warning(frmt("Function should use one of two templates:\n"
				"\tfnc entry %s() ... \n\tfnc entry %s(i32, char**) ...\n",
					func_decl->name->value, func_decl->name->value), func_decl->name->context);
		}
	}
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
		return PTR_SIZE;
	case TYPE_PRIMITIVE:
		return type->size;
	default:
		if (is_aggregate_type(type)) {
			return get_size_of_aggregate_type(type, table);
		}
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
		for (size_t i = 0; i < sbuffer_len(type->type->attrs.compound.members); i++) {
			buffer = type->attrs.compound.members[i]->type->size;
			// when type is union, calculate max between size and size of current member
			// otherwise just add member size to whole size (in case of struct)
			size = type->kind == TYPE_UNION ? 
				max(size, buffer) : (size + buffer);
		}
		return size;
	case TYPE_ARRAY:
		return (uint32_t)(get_size_of_type(type->base, table) *
			evaluate_expr_itype(type->attrs.arr.dimension));
	default:
		report_error(frmt("Passed type \'%s\' is not aggregate in function: %s.",
			type_tostr_plain(type), __FUNCTION__), NULL);
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
		for (uint32_t i = 0; i < sbuffer_len(type->members); i++) {
			type->members[i]->type->size = get_size_of_type(
				type->members[i]->type, table);
			// in case of union, dont add offset for member, 
			// keep it 0 for all members (here there are no logic for that
			// because Member struct is allocated already with 0 stored in offset)
			if (type->kind == TYPE_STRUCT) {
				type->members[i]->offset = offset;
				offset += type->members[i]->type->size;
			}
		}
		type->size = get_size_of_aggregate_type(type, table);
		break;
	}
}

void complete_type(Type* type, Table* table)
{
	/*
		Completes the missing information in type structure
		if its struct, enum or union type + gets it's size
	*/

	TableEntity* user_type = NULL;
	Type* base = get_base_type(type);

	// type void cannot be completed
	if (type->kind == TYPE_VOID) {
		return;
	}
	if (user_type = get_struct(base->repr, table)) {
		base->kind = TYPE_STRUCT;
		base->members = user_type->struct_decl->members;
	}
	else if (get_enum(base->repr, table)) {
		base->kind = TYPE_ENUM;
	}
	else if (user_type = get_union(base->repr, table)) {
		base->kind = TYPE_UNION;
		base->members = user_type->union_decl->members;
	}
}

bool is_const_expr(Expr* expr)
{
	/*
		Statically determines if specified expression
		is constant. (it means this expression can be evaluated)
	*/

	if (!expr) {
		return false;
	}
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

bool is_addressable_expr(Expr* expr)
{
	/*
		Statically determines if the specified expression 
		is addressable expression. (if we able to assign some
		value to this expression: variable, array index etc.)
	*/

	if (!expr) {
		return false;
	}
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