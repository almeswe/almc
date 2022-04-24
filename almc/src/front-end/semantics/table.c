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
		table_entities_free(table->enum_members);

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

bool add_func2(FuncDecl* func_decl, Table* table)
{
	return add_table_entity(&table->functions, (void*)func_decl,
		func_decl->name->svalue, TABLE_ENTITY_FUNCTION, table);
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

bool add_variable2(VarDecl* var_decl, Table* table)
{
	return add_table_entity(&table->locals, (void*)var_decl,
		var_decl->type_var->var, TABLE_ENTITY_VARIABLE, table);
}

bool add_label2(LabelDecl* label_decl, Table* table)
{
	return add_table_entity(&table->labels, (void*)label_decl,
		label_decl->label->svalue, TABLE_ENTITY_LABEL, table);
}

bool add_parameter(TypeVar* type_var, Table* table)
{
	return add_table_entity(&table->parameters, (void*)type_var,
		type_var->var, TABLE_ENTITY_PARAMETER, table);
}

void add_initialized_variable(char* var_name, Table* table) 
{
	TableEntity* entity;
	if (entity = get_variable(var_name, table))
		entity->is_initialized = true;
}

bool is_table_entity_declared(const char* decl_name, 
	TableEntityKind kind, Table* table)
{
	/*
		Generic function for checking if the table entity
		is declared in current scope (Table*).
		Returns true if table entity is already declared in collection, otherwise false.
	*/

#define _decld_in(c, m_in)							     \
	for (Table* p = table; p != NULL; p = p->nested_in)  \
		for (size_t i = 0; i < sbuffer_len(p->c); i++) 	 \
			if (strcmp(p->c[i]->m_in, decl_name) == 0) 	 \
				return true;

#define _decld_in_ret(c, m_in)  \
	_decld_in(c, m_in);			\
	return false
	
	switch (kind) {
		case TABLE_ENTITY_ENUM:	
		case TABLE_ENTITY_UNION:
		case TABLE_ENTITY_STRUCT:
			_decld_in(structs, struct_decl->name);
			_decld_in(enums, enum_decl->name);
			_decld_in_ret(unions, union_decl->name);
		case TABLE_ENTITY_VARIABLE:
		case TABLE_ENTITY_PARAMETER:
		case TABLE_ENTITY_ENUM_MEMBER:
			_decld_in(locals, local->type_var->var);
			_decld_in(parameters, parameter->var);
			_decld_in_ret(enum_members, enum_member->name);
		case TABLE_ENTITY_LABEL:
			_decld_in_ret(labels, label->label->svalue);
		case TABLE_ENTITY_FUNCTION:
			_decld_in_ret(functions, function->name->svalue);
		default:
			report_error(frmt("Unknown table entity kind met"
				" in function: %s", __FUNCTION__), NULL);
#undef _decld_in
#undef _decld_in_ret
	}
	return false;
}

bool add_table_entity(TableEntity*** entities, void* decl, 
	const char* decl_name, TableEntityKind kind, Table* table)
{
	/*
		Generic function for appending table entity to 
		collection if it is not already declared there.
		Returns true if entity was successfully appended, otherwise false.
	*/

#define _set_as(as) \
	entity->as = decl; break

	bool declared = false;
	if (!(declared = is_table_entity_declared(decl_name, kind, table))) {
		TableEntity* entity = table_entity_new(kind);
		switch (kind) {
			case TABLE_ENTITY_ENUM:			_set_as(enum_decl);
			case TABLE_ENTITY_UNION:		_set_as(union_decl);
			case TABLE_ENTITY_STRUCT:		_set_as(struct_decl);
			case TABLE_ENTITY_VARIABLE:		_set_as(local);
			case TABLE_ENTITY_PARAMETER:	_set_as(parameter);
			case TABLE_ENTITY_LABEL:		_set_as(label);
			case TABLE_ENTITY_FUNCTION:		_set_as(function);
			case TABLE_ENTITY_ENUM_MEMBER:	_set_as(enum_member);
			default:
				report_error(frmt("Unknown table entity kind met"
					" in function: %s.", __FUNCTION__), NULL);
		}
		sbuffer_add(*entities, entity);
#undef _set_as
	}
	return !declared;
}

bool add_enum_member(EnumMember* enum_member, Table* table)
{
	return add_table_entity(&table->enum_members, (void*)enum_member,
		enum_member->name, TABLE_ENTITY_ENUM_MEMBER, table);
}

bool add_enum2(EnumDecl* enum_decl, Table* table)
{
	return add_table_entity(&table->enums, (void*)enum_decl, 
		enum_decl->name, TABLE_ENTITY_ENUM, table);
}

bool add_struct2(StructDecl* struct_decl, Table* table)
{
	return add_table_entity(&table->structs, (void*)struct_decl,
		struct_decl->name, TABLE_ENTITY_STRUCT, table);
}

bool add_union2(UnionDecl* union_decl, Table* table)
{
	return add_table_entity(&table->unions, (void*)union_decl,
		union_decl->name, TABLE_ENTITY_UNION, table);
}

TableEntity* get_table_entity(const char* entity_name,
	TableEntityKind kind, Table* table)
{
	/*
		Generic function for retrieving table entity by name.
	*/

#define _get_from(c, m_in)								 \
	for (Table* p = table; p!= NULL; p = p->nested_in)	 \
		for (size_t i = 0; i < sbuffer_len(p->c); i++) 	 \
			if (strcmp(p->c[i]->m_in, entity_name) == 0) \
				return p->c[i];							 \
	return NULL

	// Validation of table entity kind
	if (kind < TABLE_ENTITY_ENUM || 
		kind > TABLE_ENTITY_FUNCTION) {
			return NULL;
	}
	switch (kind) {
		case TABLE_ENTITY_ENUM:			_get_from(enums, enum_decl->name);
		case TABLE_ENTITY_UNION:		_get_from(unions, union_decl->name);
		case TABLE_ENTITY_STRUCT:		_get_from(structs, struct_decl->name);
		case TABLE_ENTITY_VARIABLE:		_get_from(locals, local->type_var->var);
		case TABLE_ENTITY_PARAMETER:	_get_from(parameters, parameter->var);
		case TABLE_ENTITY_LABEL:		_get_from(labels, label->label->svalue);
		case TABLE_ENTITY_FUNCTION:		_get_from(functions, function->name->svalue);
		case TABLE_ENTITY_ENUM_MEMBER:	_get_from(enum_members, enum_member->name);
		default:
			report_error(frmt("Unknown table entity kind met"
				" in function: %s.", __FUNCTION__), NULL);
	}
	return NULL;
#undef _get_from
}

TableEntity* get_enum_member(const char* enum_member_name, Table* table)
{
	return get_table_entity(enum_member_name, 
		TABLE_ENTITY_ENUM_MEMBER , table);
}

TableEntity* get_parameter(const char* parameter_name, Table* table)
{
	return get_table_entity(parameter_name,
		TABLE_ENTITY_PARAMETER, table);
}

TableEntity* get_variable(const char* var_name, Table* table)
{
	return get_table_entity(var_name,
		TABLE_ENTITY_VARIABLE, table);
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