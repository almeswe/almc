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
	Table* table = cnew(Table, 1);
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
	TableEntity* entity = cnew(TableEntity, 1);
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

//typedef void (ent_add_func)(void*, void*, TableEntityKind, Table*);
//typedef bool (ent_decld_func)(const char*, Table*);

bool is_table_entity_declared(const char* decl_name, 
	TableEntityKind kind, Table* table)
{
	/*
		Generic function for checking if the table entity
		is declared in currenct scope (Table*).
		Returns true if table entitt is alread declared in collection, otherwise false.
	*/

	switch (kind) {
#define _decld_in(c, m_in)						 \
	for (size_t i = 0; i < sbuffer_len(c); i++)	 \
		if (strcmp(c[i]->m_in, decl_name) == 0)  \
			return true;						 \
	return false
		case TABLE_ENTITY_ENUM:		
			_decld_in(table->enums, enum_decl->name);
		case TABLE_ENTITY_UNION:		
			_decld_in(table->unions, union_decl->name);
		case TABLE_ENTITY_STRUCT:		
			_decld_in(table->structs, struct_decl->name);
		case TABLE_ENTITY_VARIABLE:		
			_decld_in(table->locals, local->type_var->var);
		case TABLE_ENTITY_PARAMETER:	
			_decld_in(table->parameters, parameter->var);
		case TABLE_ENTITY_LABEL:		
			_decld_in(table->labels, label->label->svalue);
		case TABLE_ENTITY_FUNCTION:		
			_decld_in(table->functions, function->name->svalue);
		default:
			report_error("Unknown table entity kind met in is_table_entity_declared"
				" this is bug actually.", NULL);
#undef _decld_in
	}
	return false;
}

bool add_table_entity(TableEntity*** entities, void* decl, 
	const char* decl_name, TableEntityKind kind, Table* table)
{
	/*
		Generic function appending table entity to 
		collection if it is not alreadt declared there.
		Returns true if entity was successfully appended, otherwise false.
	*/

	bool declared = false;
	if (!(declared = is_table_entity_declared(decl_name, kind, table))) {
		TableEntity* entity = table_entity_new(kind);
		switch (kind) {
#define _set_as(as) \
	entity->as = decl; break
			case TABLE_ENTITY_ENUM:
				_set_as(enum_decl);
			case TABLE_ENTITY_UNION:
				_set_as(union_decl);
			case TABLE_ENTITY_STRUCT:
				_set_as(struct_decl);
			case TABLE_ENTITY_VARIABLE:
				_set_as(local);
			case TABLE_ENTITY_PARAMETER:
				_set_as(parameter);
			case TABLE_ENTITY_LABEL:
				_set_as(label);
			case TABLE_ENTITY_FUNCTION:
				_set_as(function);
			default:
				report_error("Unknown table entity kind met in add_table_entity"
					" this is bug actually.", NULL);
#undef _set_as
		}
		sbuffer_add(*entities, entity);
	}
	return !declared;
}

bool add_enum2(EnumDecl* enum_decl, Table* table)
{
	return add_table_entity(&table->enums, (void*)enum_decl, 
		enum_decl->name, TABLE_ENTITY_ENUM, table);
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