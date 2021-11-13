#include "visitor.h"

#define IS_USER_TYPE_ALREADY_DECLARED(typedec, typestr) \
	if (is_##typedec##_declared(stmts[i]->type_decl->##typedec##_decl->##typedec##_name, table)) \
		report_error(frmt("%s type %s is already declared.",						             \
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
	case STMT_EXPR:
		visit_expr(stmt->expr_stmt->expr, table);
		break;
	case STMT_BLOCK:
		visit_block(stmt->block, table_new(table));
		break;
	case STMT_VAR_DECL:
		visit_var_decl(stmt->var_decl, table);
		break;
	case STMT_TYPE_DECL:
		visit_type_decl(stmt->type_decl, table);
		break;
	case STMT_FUNC_DECL:
		visit_func_decl(stmt->func_decl, table);
		break;
	default:
		report_error("Unknown statement kind to visit.", NULL);
	}
}

void visit_type(Type* type, Table* table)
{
	if (!type->mods.is_predefined)
		if (!is_struct_declared(type->repr, table) &&
			!is_union_declared(type->repr, table) &&
			!is_enum_declared(type->repr, table))
				report_error2(frmt("Undefined type %s met.",
					type->repr), type->area);
	if (type->mods.is_array && type->mods.is_ptr)
		assert(0);
}

void visit_non_void_type(Type* type, Table* table)
{
	if (type->mods.is_void && !type->mods.is_ptr)
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
			if (is_function_declared(stmts[i]->func_decl->func_name, table))
				report_error(frmt("Function %s is already declared.",
					stmts[i]->func_decl->func_name), NULL);
			add_function(stmts[i]->func_decl, table);
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
		visit_idnt(expr->idnt, table);
		break;
	case EXPR_UNARY_EXPR:
		visit_unary_expr(expr, table);
		break;
	case EXPR_BINARY_EXPR:
		visit_binary_expr(expr, table);
		break;
	default:
		assert(0);
	}
}

void visit_idnt(Idnt* idnt, Table* table)
{
	// checking if identifier is parameter, then just set its type to idnt
	if (is_function_param_passed(idnt->svalue, table))
		idnt->type = get_function_param(idnt->svalue, table)->type;
	else
	{
		// rather check if declared and initialized
		if (!is_variable_declared(idnt->svalue, table))
			report_error(frmt("Variable %s is not declared.",
				idnt->svalue), idnt->context);
		if (!is_variable_initialized(idnt->svalue, table))
			report_error(frmt("Variable %s is not initialized in current scope.",
				idnt->svalue), idnt->context);
		idnt->type = get_variable(idnt->svalue, table)->type_var->type;
	}
}

void visit_unary_expr(Expr* expr, Table* table)
{
	switch (expr->unary_expr->kind)
	{
	case UNARY_DEREFERENCE:
		if (!is_addressable_value(expr))
			report_error2(frmt("Expression is not addressible value, cannot dereference it."), 
				expr->unary_expr->area);
		visit_expr(expr->unary_expr->expr, table);
		break;
	default:
		assert(0);
	}
}

void visit_arr_member_accessor_expr(BinaryExpr* binary_expr, Table* table)
{
	if (!is_addressable_value(binary_expr->lexpr, table))
		report_error2(frmt("Expression is not addressible value, cannot access it like array."), 
			binary_expr->area);
	visit_expr(binary_expr->lexpr, table);
	visit_expr(binary_expr->rexpr, table);
	// todo: check for array type
}

void visit_binary_expr(Expr* expr, Table* table)
{
	//todo: add type determinated logic for shifts, bitwises etc.

	switch (expr->binary_expr->kind)
	{
	case BINARY_ADD:
	case BINARY_SUB:
	case BINARY_DIV:
	case BINARY_MOD:
	case BINARY_MULT:
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
		visit_expr(expr->binary_expr->lexpr, table);
		visit_expr(expr->binary_expr->rexpr, table);
		break;

	case BINARY_ARR_MEMBER_ACCESSOR:
		visit_arr_member_accessor_expr(
			expr->binary_expr, table);
		break;
	default:
		assert(0);
	}
}

void visit_var_decl(VarDecl* var_decl, Table* table)
{
	if (is_variable_declared(var_decl->type_var->var, table))
		report_error2(frmt("Variable %s is already declared.",
			var_decl->type_var->var), var_decl->type_var->area);
	else if (is_function_param_passed(var_decl->type_var->var, table))
		report_error2(frmt("%s is already declared as function's parameter.",
			var_decl->type_var->var), var_decl->type_var->area);
	else
	{
		add_variable(var_decl, table);
		visit_non_void_type(var_decl->type_var->type, table);
		if (var_decl->var_init)
		{
			visit_expr(var_decl->var_init, table);
			add_initialized_variable(var_decl->type_var->var, table);

			// check type of created variable with type of initializing expression
			Type* init_expr_type = get_expr_type(var_decl->var_init, table);
			if (!can_cast_implicitly(var_decl->type_var->type, init_expr_type))
				report_error2(frmt("Expression-initializer has incompatible type %s with type of variable %s.",
					type_tostr_plain(init_expr_type), type_tostr_plain(var_decl->type_var->type)), 
						get_expr_area(var_decl->var_init));
		}
	}
}

void visit_enum(EnumDecl* enum_decl, Table* table)
{
	for (size_t i = 0; i < sbuffer_len(enum_decl->enum_idnts); i++)
	{
		// iterating through all members of all enums (except current enum)
		for (Table* parent = table; parent != NULL; parent = parent->parent)					    // each scope from above scope
			for (size_t j = 0; j < sbuffer_len(parent->enums); j++)								    // each enum in iterating scope
				if (strcmp(parent->enums[j]->enum_name, enum_decl->enum_name) != 0)					// if iterating enum != current enum
					for (size_t z = 0; z < sbuffer_len(parent->enums[j]->enum_idnts); z++)			// each member in iterating enum
						if (strcmp(parent->enums[j]->enum_idnts[z]->svalue,
								   enum_decl->enum_idnts[i]->svalue) == 0)
							report_error(frmt("Enum member %s is already declared in %s enum.",
								enum_decl->enum_idnts[i]->svalue, parent->enums[j]->enum_name), 
									enum_decl->enum_idnts[i]->context);

		// now checking for any duplicated names in current enum
		for (size_t g = i + 1; g < sbuffer_len(enum_decl->enum_idnts); g++)
			if (strcmp(enum_decl->enum_idnts[i]->svalue, enum_decl->enum_idnts[g]->svalue) == 0)
				report_error(frmt("Member %s is already declared in %s enum declaration.",
					enum_decl->enum_idnts[i]->svalue, enum_decl->enum_name), enum_decl->enum_idnts[i]->context);
		//todo: check for initizers (only consts)
	}
}

void visit_union(UnionDecl* union_decl, Table* table)
{
	for (size_t i = 0; i < sbuffer_len(union_decl->union_mmbrs); i++)
	{
		// checking for self included type (and not pointer)
		if (strcmp(union_decl->union_name, union_decl->union_mmbrs[i]->type->repr) == 0)
			if (!union_decl->union_mmbrs[i]->type->mods.is_ptr)
				report_error(frmt("Member %s has type %s which is self included, and not pointer.",
					union_decl->union_mmbrs[i]->var, union_decl->union_name), 
						union_decl->union_mmbrs[i]->area);
		visit_non_void_type(union_decl->union_mmbrs[i]->type, table);
	}

	// checking here for duplicated members
	for (size_t i = 0; i < sbuffer_len(union_decl->union_mmbrs); i++)
		for (size_t j = i + 1; j < sbuffer_len(union_decl->union_mmbrs); j++)
			if (strcmp(union_decl->union_mmbrs[i]->var, union_decl->union_mmbrs[j]->var) == 0)
				report_error(frmt("Member [%s] is already declared in [%s] union declaration.",
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
				report_error(frmt("Member %s has type %s which is self included, and not pointer.",
					struct_decl->struct_mmbrs[i]->var, struct_decl->struct_name), 
						struct_decl->struct_mmbrs[i]->area);
		visit_non_void_type(struct_decl->struct_mmbrs[i]->type, table);
	}

	// checking here for duplicated members
	for (size_t i = 0; i < sbuffer_len(struct_decl->struct_mmbrs); i++)
		for (size_t j = i+1; j < sbuffer_len(struct_decl->struct_mmbrs); j++)
			if (strcmp(struct_decl->struct_mmbrs[i]->var, struct_decl->struct_mmbrs[j]->var) == 0)
				report_error(frmt("Member %s is already declared in %s struct declaration.",
					struct_decl->struct_mmbrs[i]->var, struct_decl->struct_name), 
						struct_decl->struct_mmbrs[i]->area);
}

void visit_type_decl(TypeDecl* type_decl, Table* table)
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
	}
}

void visit_block(Block* block, Table* table)
{
	for (size_t i = 0; i < sbuffer_len(block->stmts); i++)
		visit_stmt(block->stmts[i], table);
}

void visit_func_decl(FuncDecl* func_decl, Table* table)
{
	Table* local = table_new(table);

	if (func_decl->func_spec.is_external &&
		func_decl->func_spec.is_forward &&
		func_decl->func_spec.is_intrinsic)
			assert(0);

	if (func_decl->func_body)
		visit_scope(func_decl->func_body->stmts, local);

	// todo: check for main func 
	// checking func parameters
	for (size_t i = 0; i < sbuffer_len(func_decl->func_params); i++)
	{
		// checking for duplicated parameter specification
		if (is_function_param_passed(func_decl->func_params[i]->var, local))
			report_error2(frmt("Function parameter %s is already specified in %s function parameter list.",
				func_decl->func_params[i]->var, func_decl->func_name), func_decl->func_params[i]->area);
		add_function_param(func_decl->func_params[i], local);

		// checking for void type
		visit_non_void_type(func_decl->func_params[i]->type, table);
	}

	//todo: add all parameters to variable table
	//for (size_t i = 0; i < sbuffer_len(func_decl->func_params); i++);

	//checking function's return type
	visit_type(func_decl->func_type, table);
	//checking function's body
	visit_block(func_decl->func_body, local);
}

int expr_contains_var(Expr* expr)
{
	switch (expr->kind)
	{
	case EXPR_IDNT:
		return 1;
	case EXPR_UNARY_EXPR:
		return expr_contains_var(expr->unary_expr->expr);
	case EXPR_BINARY_EXPR:
	case EXPR_TERNARY_EXPR:
		return expr_contains_var(expr->binary_expr->lexpr) ||
			   expr_contains_var(expr->binary_expr->rexpr);
	default:
		return 0;
	}
}

int is_addressable_value(Expr* expr)
{
	if (!expr)
		return 0;
	switch (expr->kind)
	{	
	case EXPR_IDNT:
		return 1;
	case EXPR_UNARY_EXPR:
		switch (expr->unary_expr->kind)
		{
		case UNARY_PREFIX_INC:
		case UNARY_PREFIX_DEC:
		case UNARY_POSTFIX_INC:
		case UNARY_POSTFIX_DEC:
			return expr->unary_expr->kind == EXPR_IDNT;

		case UNARY_DEREFERENCE:
			switch (expr->unary_expr->expr->kind)
			{
			case EXPR_IDNT:
				return 1;
			case EXPR_UNARY_EXPR:
				return is_addressable_value(expr->unary_expr->expr);
			case EXPR_BINARY_EXPR:
				return expr_contains_var(expr->unary_expr->expr->binary_expr->lexpr) ||
					   expr_contains_var(expr->unary_expr->expr->binary_expr->rexpr);
			case EXPR_TERNARY_EXPR:
				return expr_contains_var(expr->unary_expr->expr->ternary_expr->lexpr) &&
					   expr_contains_var(expr->unary_expr->expr->ternary_expr->rexpr);
			default:
				return 0;
			}
			break;
		}
		break;
	case EXPR_BINARY_EXPR:
		switch (expr->binary_expr->kind)
		{
		case BINARY_MEMBER_ACCESSOR:
		case BINARY_PTR_MEMBER_ACCESSOR:
			return is_addressable_value(expr->binary_expr->lexpr) &&
				   is_addressable_value(expr->binary_expr->rexpr);
		case BINARY_ARR_MEMBER_ACCESSOR:
			return is_addressable_value(expr->binary_expr->lexpr);
		default:
			return 0;
		}
		break;
	case EXPR_TERNARY_EXPR:
		return is_addressable_value(expr->ternary_expr->lexpr) &&
			is_addressable_value(expr->ternary_expr->rexpr);
	default:
		return 0;
	}
	// p 
	// p[i]
	// p.i
	// p->i
	// *(p+i)
	// p++ p-- 
	// --p ++p

	// p->c->d
}