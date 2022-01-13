#include "table.h"

#define is_declared_in_collection(member, member_in_collection, collection)   \
	for (size_t i = 0; i < sbuffer_len(collection); i++)				      \
		if (strcmp(collection[i]->member_in_collection, member) == 0)         \
			return true;													  \
	return false;

#define get_from_collection(member, member_in_collection, collection)   \
	for (size_t i = 0; i < sbuffer_len(collection); i++)                \
		if (strcmp(collection[i]->member_in_collection, member) == 0)   \
			return collection[i];						                \
	return NULL;

Table* table_new(Table* parent)
{
	Table* table = cnew_s(Table, table, 1);
	table->nested_in = parent;

	if (parent)
	{
		table->in_loop = parent->in_loop;
		table->in_switch = parent->in_switch;
		table->in_function = parent->in_function;
		sbuffer_add(parent->nesting, table);
	}
	return table;
}

void table_free(Table* table)
{
	if (table)
	{
		table_entities_free(table->functions);
		table_entities_free(table->locals);
		table_entities_free(table->parameters);
		table_entities_free(table->labels);

		table_entities_free(table->enums);
		table_entities_free(table->unions);
		table_entities_free(table->structs);

		for (size_t i = 0; i < sbuffer_len(table->nesting); i++)
			table_free(table->nesting[i]);
		sbuffer_free(table->nesting);
		free(table);
	}
}

TableEntity* table_entity_new(TableEntityKind kind)
{
	TableEntity* entity = cnew_s(TableEntity, entity, 1);
	return entity->kind = kind, entity;
}

void table_entity_free(TableEntity* entity)
{
	if (entity)
		free(entity);
}

void table_entities_free(TableEntity** entities)
{
	for (size_t i = 0; i < sbuffer_len(entities); i++)
		table_entity_free(entities[i]);
	sbuffer_free(entities);
}

bool is_function_declared(const char* func_name, Table* table)
{
	for (Table* parent = table; parent != NULL; parent = parent->nested_in)
		is_declared_in_collection(func_name, function->name->svalue, parent->functions);
}

bool is_variable_declared(const char* var_name, Table* table)
{
	for (Table* parent = table; parent != NULL; parent = parent->nested_in)
		is_declared_in_collection(var_name, local->type_var->var, parent->locals);
}

bool is_function_param_passed(const char* param_name, Table* table)
{
	for (Table* parent = table; parent != NULL; parent = parent->nested_in)
		is_declared_in_collection(param_name, parameter->var, parent->parameters);
}

bool is_label_declared(const char* label_name, Table* table)
{
	for (Table* parent = table; parent != NULL; parent = parent->nested_in)
		is_declared_in_collection(label_name, label->label->svalue, parent->labels);
}

bool is_variable_initialized(const char* var_name, Table* table)
{
	for (Table* parent = table; parent != NULL; parent = parent->nested_in)
		for (size_t i = 0; i < sbuffer_len(parent->locals); i++)
			if (strcmp(parent->locals[i], var_name) == 0 && parent->locals[i]->is_initialized)
				return true;
	return false;
}

bool is_enum_declared(const char* enum_name, Table* table)
{
	for (Table* parent = table; parent != NULL; parent = parent->nested_in)
		is_declared_in_collection(enum_name, enum_decl->name, parent->enums);
}

bool is_struct_declared(const char* struct_name, Table* table)
{
	for (Table* parent = table; parent != NULL; parent = parent->nested_in)
		is_declared_in_collection(struct_name, struct_decl->name, parent->structs);
}

bool is_union_declared(const char* union_name, Table* table)
{
	for (Table* parent = table; parent != NULL; parent = parent->nested_in)
		is_declared_in_collection(union_name, union_decl->name, parent->unions);
}

void add_function(FuncDecl* func_decl, Table* table)
{
	if (!is_function_declared(func_decl->name->svalue, table))
	{
		TableEntity* entity = table_entity_new(TABLE_ENTITY_FUNCTION);
		entity->function = func_decl;
		sbuffer_add(table->functions, entity);
	}
}

void add_variable(VarDecl* var_decl, Table* table)
{
	if (!is_variable_declared(var_decl->type_var->var, table))
	{
		TableEntity* entity = table_entity_new(TABLE_ENTITY_VARIABLE);
		entity->local = var_decl;
		sbuffer_add(table->locals, entity);
	}
}

void add_label(LabelDecl* label_decl, Table* table)
{
	if (!is_label_declared(label_decl->label->svalue, table))
	{
		TableEntity* entity = table_entity_new(TABLE_ENTITY_LABEL);
		entity->label = label_decl;
		sbuffer_add(table->labels, entity);
	}
}

void add_function_param(TypeVar* type_var, Table* table)
{
	if (!is_function_param_passed(type_var->var, table))
	{
		TableEntity* entity = table_entity_new(TABLE_ENTITY_PARAMETER);
		entity->label = type_var;
		sbuffer_add(table->parameters, entity);
	}
}

void add_initialized_variable(char* var_name, Table* table)
{
	TableEntity* entity;
	if (entity = get_variable(var_name, table))
		entity->is_initialized = true;
}

void add_enum(EnumDecl* enum_decl, Table* table)
{
	if (!is_enum_declared(enum_decl->name, table))
	{
		TableEntity* entity = table_entity_new(TABLE_ENTITY_ENUM);
		entity->enum_decl = enum_decl;
		sbuffer_add(table->enums, entity);
	}
}

void add_struct(StructDecl* struct_decl, Table* table)
{
	if (!is_struct_declared(struct_decl->name, table))
	{
		TableEntity* entity = table_entity_new(TABLE_ENTITY_STRUCT);
		entity->struct_decl = struct_decl;
		sbuffer_add(table->structs, entity);
	}
}

void add_union(UnionDecl* union_decl, Table* table)
{
	if (!is_union_declared(union_decl->name, table))
	{
		TableEntity* entity = table_entity_new(TABLE_ENTITY_UNION);
		entity->union_decl = union_decl;
		sbuffer_add(table->unions, entity);
	}
}

TableEntity* get_variable(const char* var_name, Table* table)
{
	for (Table* parent = table; parent != NULL; parent = parent->nested_in)
		get_from_collection(var_name, local->type_var->var, parent->locals);
}

TableEntity* get_label(const char* label_name, Table* table)
{
	for (Table* parent = table; parent != NULL; parent = parent->nested_in)
		get_from_collection(label_name, label->label->svalue, parent->labels);
}

TableEntity* get_function(const char* func_name, Table* table)
{
	for (Table* parent = table; parent != NULL; parent = parent->nested_in)
		get_from_collection(func_name, function->name->svalue, parent->functions);
}

TableEntity* get_function_param(const char* param_name, Table* table)
{
	for (Table* parent = table; parent != NULL; parent = parent->nested_in)
		get_from_collection(param_name, parameter->var, parent->parameters);
}

TableEntity* get_enum(const char* enum_name, Table* table)
{
	for (Table* parent = table; parent != NULL; parent = parent->nested_in)
		get_from_collection(enum_name, enum_decl->name, parent->enums);
}

TableEntity* get_union(const char* union_name, Table* table)
{
	for (Table* parent = table; parent != NULL; parent = parent->nested_in)
		get_from_collection(union_name, union_decl->name, parent->unions);
}

TableEntity* get_struct(const char* struct_name, Table* table)
{
	for (Table* parent = table; parent != NULL; parent = parent->nested_in)
		get_from_collection(struct_name, struct_decl->name, parent->structs);
}