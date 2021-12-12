#include "table.h"

//todo: switch all this stuff from arrays to hash-tables

#define is_declared_in_collection(member, member_in_collection, collection)   \
	for (size_t i = 0; i < sbuffer_len(collection); i++)				      \
		if (strcmp(collection[i]->member_in_collection, member) == 0)         \
			return 1;													      \
	return 0;

#define get_from_collection(member, member_in_collection, collection)   \
	for (size_t i = 0; i < sbuffer_len(collection); i++)                \
		if (strcmp(collection[i]->member_in_collection, member) == 0)   \
			return collection[i];						                \
	return NULL;

Table* table_new(Table* parent)
{
	Table* table = new_s(Table, table);
	table->functions = NULL;
	table->variables = NULL;
	table->parameters = NULL;
	table->labels = NULL;

	table->enums = NULL;
	table->unions = NULL;
	table->structs = NULL;

	table->in_function = parent ?
		parent->in_function : NULL;

	table->in_switch = parent ?
		parent->in_switch : NULL;
	table->in_loop = parent ?
		parent->in_loop : NULL;

	table->parent = parent;
	table->childs = NULL;

	table->initialized_variables_in_scope = NULL;

	if (parent)
		sbuffer_add(parent->childs, table);
	return table;
}

void table_free(Table* table)
{
	if (table)
	{
		sbuffer_free(table->functions);
		sbuffer_free(table->variables);
		sbuffer_free(table->parameters);
		sbuffer_free(table->labels);

		sbuffer_free(table->enums);
		sbuffer_free(table->unions);
		sbuffer_free(table->structs);

		for (size_t i = 0; i < sbuffer_len(table->childs); i++)
			table_free(table->childs[i]);
		sbuffer_free(table->childs);
		for (size_t i = 0; i < sbuffer_len(table->initialized_variables_in_scope); i++)
			free(table->initialized_variables_in_scope[i]);
		sbuffer_free(table->initialized_variables_in_scope);
		free(table);
	}
}

int is_function_declared(const char* func_name, Table* table)
{
	for (Table* parent = table; parent != NULL; parent = parent->parent)
		is_declared_in_collection(func_name, func_name->svalue, parent->functions);
}

int is_variable_declared(const char* var_name, Table* table)
{
	for (Table* parent = table; parent != NULL; parent = parent->parent)
		is_declared_in_collection(var_name, type_var->var, parent->variables);
}

int is_function_param_passed(const char* param_name, Table* table)
{
	for (Table* parent = table; parent != NULL; parent = parent->parent)
		is_declared_in_collection(param_name, var, parent->parameters);
}

int is_label_declared(const char* label_name, Table* table)
{
	for (Table* parent = table; parent != NULL; parent = parent->parent)
		is_declared_in_collection(label_name, label_idnt->svalue, parent->labels);
}

int is_variable_initialized(const char* var_name, Table* table)
{
	for (Table* parent = table; parent != NULL; parent = parent->parent)
		for (size_t i = 0; i < sbuffer_len(parent->initialized_variables_in_scope); i++)
			if (strcmp(parent->initialized_variables_in_scope[i], var_name) == 0)
				return 1;
	return 0;
}

int is_enum_declared(const char* enum_name, Table* table)
{
	for (Table* parent = table; parent != NULL; parent = parent->parent)
		is_declared_in_collection(enum_name, name, parent->enums);
}

int is_struct_declared(const char* struct_name, Table* table)
{
	for (Table* parent = table; parent != NULL; parent = parent->parent)
		is_declared_in_collection(struct_name, name, parent->structs);
}

int is_union_declared(const char* union_name, Table* table)
{
	for (Table* parent = table; parent != NULL; parent = parent->parent)
		is_declared_in_collection(union_name, name, parent->unions);
}

void add_function(FuncDecl* func_decl, Table* table)
{
	if (!is_function_declared(func_decl->func_name->svalue, table))
		sbuffer_add(table->functions, func_decl);
}

void add_variable(VarDecl* var_decl, Table* table)
{
	if (!is_variable_declared(var_decl->type_var->var, table))
		sbuffer_add(table->variables, var_decl);
}

void add_label(LabelDecl* label_decl, Table* table)
{
	if (!is_label_declared(label_decl->label_idnt->svalue, table))
		sbuffer_add(table->labels, label_decl);
}

void add_function_param(TypeVar* type_var, Table* table)
{
	if (!is_function_param_passed(type_var->var, table))
		sbuffer_add(table->parameters, type_var);
}

void add_initialized_variable(char* var_name, Table* table)
{
	sbuffer_add(table->initialized_variables_in_scope, var_name);
}

void add_enum(EnumDecl* enum_decl, Table* table)
{
	if (!is_enum_declared(enum_decl->name, table))
		sbuffer_add(table->enums, enum_decl);
}

void add_struct(StructDecl* struct_decl, Table* table)
{
	if (!is_struct_declared(struct_decl->name, table))
		sbuffer_add(table->structs, struct_decl);
}

void add_union(UnionDecl* union_decl, Table* table)
{
	if (!is_union_declared(union_decl->name, table))
		sbuffer_add(table->unions, union_decl);
}

VarDecl* get_variable(const char* var_name, Table* table)
{
	for (Table* parent = table; parent != NULL; parent = parent->parent)
		get_from_collection(var_name, type_var->var, parent->variables);
}

LabelDecl* get_label(const char* label_name, Table* table)
{
	for (Table* parent = table; parent != NULL; parent = parent->parent)
		get_from_collection(label_name, label_idnt->svalue, parent->labels);
}

FuncDecl* get_function(const char* func_name, Table* table)
{
	for (Table* parent = table; parent != NULL; parent = parent->parent)
		get_from_collection(func_name, func_name->svalue, parent->functions);
}

TypeVar* get_function_param(const char* param_name, Table* table)
{
	for (Table* parent = table; parent != NULL; parent = parent->parent)
		get_from_collection(param_name, var, parent->parameters);
}

EnumDecl* get_enum(const char* enum_name, Table* table)
{
	for (Table* parent = table; parent != NULL; parent = parent->parent)
		get_from_collection(enum_name, name, parent->enums);
}

UnionDecl* get_union(const char* union_name, Table* table)
{
	for (Table* parent = table; parent != NULL; parent = parent->parent)
		get_from_collection(union_name, name, parent->unions);
}

StructDecl* get_struct(const char* struct_name, Table* table)
{
	for (Table* parent = table; parent != NULL; parent = parent->parent)
		get_from_collection(struct_name, name, parent->structs);
}