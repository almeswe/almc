#include "visitor.h"
#include "assert.h"

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
	switch (stmt->type)
	{
	case STMT_VAR_DECL:
		visit_var_decl(stmt->var_decl, table);
		break;
	case STMT_TYPE_DECL:
		visit_type_decl(stmt->type_decl, table);
		break;
	default:
		assert(0);
	}
}

void visit_type(Type* type, Table* table)
{
	if (!type->mods.is_predefined)
		if (!is_struct_declared(type->repr, table) &&
			!is_union_declared(type->repr, table) &&
			!is_enum_declared(type->repr, table))
				report_error(frmt("Undefined type [%s] met.",
					type->repr), NULL);
	if (type->mods.is_array && type->mods.is_ptr)
		assert(0);
}

void visit_scope(Stmt** stmts, Table* table)
{
	for (size_t i = 0; i < sbuffer_len(stmts); i++)
	{
		switch (stmts[i]->type)
		{
		case STMT_TYPE_DECL:
			switch (stmts[i]->type_decl->type)
			{
			case TYPE_DECL_ENUM:
				if (is_enum_declared(stmts[i]->type_decl->enum_decl->enum_name, table))
					report_error(frmt("Enum type [%s] is already declared.",
						stmts[i]->type_decl->enum_decl->enum_name), NULL);
				add_enum(stmts[i]->type_decl->enum_decl, table);
				break;
			case TYPE_DECL_UNION:
				if (is_union_declared(stmts[i]->type_decl->union_decl->union_name, table))
					report_error(frmt("Union type [%s] is already declared.",
						stmts[i]->type_decl->union_decl->union_name), NULL);
				add_union(stmts[i]->type_decl->union_decl, table);
				break;
			case TYPE_DECL_STRUCT:
				if (is_struct_declared(stmts[i]->type_decl->struct_decl->struct_name, table))
					report_error(frmt("Struct type [%s] is already declared.",
						stmts[i]->type_decl->struct_decl->struct_name), NULL);
				add_struct(stmts[i]->type_decl->struct_decl, table);
				break;
			}
			break;
		case STMT_FUNC_DECL:
			assert(0);
			break;
		}
	}
}

void visit_expr(Expr* expr, Table* table)
{
	assert(0);
}

void visit_var_decl(VarDecl* var_decl, Table* table)
{
	//todo: add type check
	if (is_variable_declared(var_decl->type_var->var, table))
		report_error(frmt("Variable [%s] is already declared.",
			var_decl->type_var->var), NULL);
	else
	{
		add_variable(var_decl, table);
		if (var_decl->type_var->type->mods.is_void && 
			!var_decl->type_var->type->mods.is_ptr)
			report_error(frmt("Cannot declare variable with void type."), NULL);
		visit_type(var_decl->type_var->type, table);
		if (var_decl->var_init)
			visit_expr(var_decl->var_init, table);
	}
}

void visit_enum(EnumDecl* enum_decl, Table* table)
{
	for (size_t i = 0; i < sbuffer_len(enum_decl->enum_idnts); i++)
	{
		// iterating through all members of all enums (except current enum)
		for (Table* parent = table; parent != NULL; parent = table->parent)							// each scope from above scope
			for (size_t j = 0; j < sbuffer_len(parent->enums); j++)								    // each enum in iterating scope
				if (strcmp(parent->enums[j]->enum_name, enum_decl->enum_name) != 0)					// if iterating enum != current enum
					for (size_t z = 0; z < sbuffer_len(parent->enums[j]->enum_idnts); z++)			// each member in iterating enum
						if (strcmp(parent->enums[j]->enum_idnts[z]->svalue,
								   enum_decl->enum_idnts[i]->svalue) == 0)
							report_error(frmt("Enum member [%s] is already declared in [%s] enum.",
								enum_decl->enum_idnts[i]->svalue, parent->enums[j]->enum_name), 
									enum_decl->enum_idnts[i]->context);

		// now checking for any duplicated names in current enum
		for (size_t g = i + 1; g < sbuffer_len(enum_decl->enum_idnts); g++)
			if (strcmp(enum_decl->enum_idnts[i]->svalue, enum_decl->enum_idnts[g]->svalue) == 0)
				report_error(frmt("Member [%s] is already declared in [%s] enum declaration.",
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
				report_error(frmt("Member [%s] has type [%s] which is self included, and not pointer.",
					union_decl->union_mmbrs[i]->var, union_decl->union_name), NULL);
		if (union_decl->union_mmbrs[i]->type->mods.is_void &&
			!union_decl->union_mmbrs[i]->type->mods.is_ptr)
			report_error(frmt("Cannot declare union member with void type."), NULL);
		visit_type(union_decl->union_mmbrs[i]->type, table);
	}

	// checking here for duplicated members
	for (size_t i = 0; i < sbuffer_len(union_decl->union_mmbrs); i++)
		for (size_t j = i + 1; j < sbuffer_len(union_decl->union_mmbrs); j++)
			if (strcmp(union_decl->union_mmbrs[i]->var, union_decl->union_mmbrs[j]->var) == 0)
				report_error(frmt("Member [%s] is already declared in [%s] union declaration.",
					union_decl->union_mmbrs[i]->var, union_decl->union_name), NULL);
}

void visit_struct(StructDecl* struct_decl, Table* table)
{
	// checking all member's types
	for (size_t i = 0; i < sbuffer_len(struct_decl->struct_mmbrs); i++)
	{
		// checking for self included type (and not pointer)
		if (strcmp(struct_decl->struct_name, struct_decl->struct_mmbrs[i]->type->repr) == 0)
			if (!struct_decl->struct_mmbrs[i]->type->mods.is_ptr)
				report_error(frmt("Member [%s] has type [%s] which is self included, and not pointer.",
					struct_decl->struct_mmbrs[i]->var, struct_decl->struct_name), NULL);
		if (struct_decl->struct_mmbrs[i]->type->mods.is_void &&
			!struct_decl->struct_mmbrs[i]->type->mods.is_ptr)
			report_error(frmt("Cannot declare struct member with void type."), NULL);
		visit_type(struct_decl->struct_mmbrs[i]->type, table);
	}

	// checking here for duplicated members
	for (size_t i = 0; i < sbuffer_len(struct_decl->struct_mmbrs); i++)
		for (size_t j = i+1; j < sbuffer_len(struct_decl->struct_mmbrs); j++)
			if (strcmp(struct_decl->struct_mmbrs[i]->var, struct_decl->struct_mmbrs[j]->var) == 0)
				report_error(frmt("Member [%s] is already declared in [%s] struct declaration.",
					struct_decl->struct_mmbrs[i]->var, struct_decl->struct_name), NULL);
}

void visit_type_decl(TypeDecl* type_decl, Table* table)
{
	Table* local = table_new(NULL);
	switch (type_decl->type)
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