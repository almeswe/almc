#include "table.h"

Table* table_new(Table* parent) {
	Table* table = cnew(Table, 1);
	table->nested_in = parent;

	if (parent) {
		table->scope_refs.in_loop = parent->scope_refs.in_loop;
		table->scope_refs.in_switch = parent->scope_refs.in_switch;
		table->scope_refs.in_function = parent->scope_refs.in_function;
		sbuffer_add(parent->nesting, table);
	}
	return table;
}

void table_free(Table* table) {
	if (table) {
		table_entities_free(table->scopes.functions);
		table_entities_free(table->scopes.locals);
		table_entities_free(table->scopes.parameters);
		table_entities_free(table->scopes.labels);
		table_entities_free(table->scopes.enum_members);
		table_entities_free(table->scopes.types);

		for (size_t i = 0; i < sbuffer_len(table->nesting); i++) {
			table_free(table->nesting[i]);
		}
		sbuffer_free(table->nesting);
		free(table);
	}
}

TableEntity* table_entity_new(TableEntityKind kind) {
	TableEntity* entity = cnew(TableEntity, 1);
	return entity->kind = kind, entity;
}

void table_entity_free(TableEntity* entity) {
	if (entity) {
		free(entity);
	}
}

void table_entities_free(TableEntity** entities) {
	for (size_t i = 0; i < sbuffer_len(entities); i++) {
		table_entity_free(entities[i]);
	}
	sbuffer_free(entities);
}

bool add_func(FuncDecl* func_decl, Table* table) {
	return add_table_entity(&table->scopes.functions, (void*)func_decl,
		func_decl->name->value, TABLE_ENTITY_FUNCTION, table);
}

bool add_variable(VarDecl* var_decl, Table* table) {
	return add_table_entity(&table->scopes.locals, (void*)var_decl,
		var_decl->type_var->var, TABLE_ENTITY_VARIABLE, table);
}

bool add_label(LabelDecl* label_decl, Table* table) {
	return add_table_entity(&table->scopes.labels, (void*)label_decl,
		label_decl->name->value, TABLE_ENTITY_LABEL, table);
}

bool add_parameter(TypeVar* type_var, Table* table) {
	return add_table_entity(&table->scopes.parameters, (void*)type_var,
		type_var->var, TABLE_ENTITY_PARAMETER, table);
}

bool is_table_entity_declared(const char* decl_name, 
	TableEntityKind kind, Table* table) {
	/*
		Generic function for checking if the table entity
		is declared in current scope (Table*).
		Returns true if table entity is already declared in collection, otherwise false.
	*/

#define _decld_in(c, m_in)							     			\
	for (Table* p = table; p != NULL; p = p->nested_in) { 			\
		for (size_t i = 0; i < sbuffer_len(p->scopes.c); i++) {		\
			if (str_eq(p->scopes.c[i]->value.m_in, decl_name)) {	\
				return true;										\
			}														\
		}															\
	}	

	switch (kind) {
		case TABLE_ENTITY_TYPE:
			_decld_in(types, type->repr);
			return false;
		case TABLE_ENTITY_LABEL:
		case TABLE_ENTITY_VARIABLE:
		case TABLE_ENTITY_PARAMETER:
		case TABLE_ENTITY_ENUM_MEMBER:
			_decld_in(labels, label->name->value);
			_decld_in(parameters, parameter->var);
			_decld_in(locals, local->type_var->var);
			_decld_in(enum_members, enum_member->name);
			return false;
		case TABLE_ENTITY_FUNCTION:
			_decld_in(functions, function->name->value);
			return false;
		default:
			report_error(frmt("Unknown table entity kind met"
				" in function: %s", __FUNCTION__), NULL);
	}
	return false;
#undef _decld_in
}

bool add_table_entity(TableEntity*** entities, void* decl, 
	const char* decl_name, TableEntityKind kind, Table* table) {
	/*
		Generic function for appending table entity to 
		collection if it is not already declared there.
		Returns true if entity was successfully appended, otherwise false.
	*/

#define _set_as(as) entity->value.as = decl; break

	bool declared = false;
	if (!(declared = is_table_entity_declared(decl_name, kind, table))) {
		TableEntity* entity = table_entity_new(kind);
		switch (kind) {
			case TABLE_ENTITY_TYPE:			_set_as(type);
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

bool add_enum_member(EnumMember* enum_member, Table* table) {
	return add_table_entity(&table->scopes.enum_members, (void*)enum_member,
		enum_member->name, TABLE_ENTITY_ENUM_MEMBER, table);
}

bool add_type(Type* type, Table* table) {
	return add_table_entity(&table->scopes.types, (void*)type, 
		type->repr, TABLE_ENTITY_TYPE, table);
}

bool add_enum(EnumDecl* enum_decl, Table* table) {
	Type* enum_type = type_new(enum_decl->name->value);
	enum_type->kind = TYPE_ENUM;
	return add_type(enum_type, table);
}

bool add_struct(StructDecl* struct_decl, Table* table) {
	Type* struct_type = type_new(struct_decl->name->value);
	struct_type->kind = TYPE_STRUCT;
	struct_type->attrs.cmpd.members = struct_decl->members;
	return add_type(struct_type, table);
}

bool add_union(UnionDecl* union_decl, Table* table) {
	Type* union_type = type_new(union_decl->name->value);
	union_type->kind = TYPE_UNION;
	union_type->attrs.cmpd.members = union_decl->members;
	return add_type(union_type, table);
}

TableEntity* get_table_entity(const char* entity_name,
	TableEntityKind kind, Table* table) {
	/*
		Generic function for retrieving table entity by name.
	*/

#define _get_from(c, m_in)								 			\
	for (Table* p = table; p!= NULL; p = p->nested_in) {	 		\
		for (size_t i = 0; i < sbuffer_len(p->scopes.c); i++) { 	\
			if (str_eq(p->scopes.c[i]->value.m_in, entity_name)) { 	\
				return p->scopes.c[i];								\
			}														\
		}															\
	}							 									\
	return NULL

	// Validation of table entity kind
	if (kind < TABLE_ENTITY_TYPE || 
		kind > TABLE_ENTITY_FUNCTION) {
			return NULL;
	}
	switch (kind) {
		case TABLE_ENTITY_TYPE:			_get_from(types, type->repr);
		case TABLE_ENTITY_VARIABLE:		_get_from(locals, local->type_var->var);
		case TABLE_ENTITY_PARAMETER:	_get_from(parameters, parameter->var);
		case TABLE_ENTITY_LABEL:		_get_from(labels, label->name->value);
		case TABLE_ENTITY_FUNCTION:		_get_from(functions, function->name->value);
		case TABLE_ENTITY_ENUM_MEMBER:	_get_from(enum_members, enum_member->name);
		default:
			report_error(frmt("Unknown table entity kind met"
				" in function: %s.", __FUNCTION__), NULL);
	}
	return NULL;
#undef _get_from
}

TableEntity* get_enum_member(const char* enum_member_name, Table* table) {
	return get_table_entity(enum_member_name, 
		TABLE_ENTITY_ENUM_MEMBER , table);
}

TableEntity* get_parameter(const char* parameter_name, Table* table) {
	return get_table_entity(parameter_name,
		TABLE_ENTITY_PARAMETER, table);
}

TableEntity* get_variable(const char* var_name, Table* table) {
	return get_table_entity(var_name,
		TABLE_ENTITY_VARIABLE, table);
}

TableEntity* get_label(const char* label_name, Table* table) {
	return get_table_entity(label_name,
		TABLE_ENTITY_LABEL, table);
}

TableEntity* get_function(const char* func_name, Table* table) {
	return get_table_entity(func_name,
		TABLE_ENTITY_FUNCTION, table);
}

TableEntity* get_type(const char* type_name, Table* table) {
	return get_table_entity(type_name,
		TABLE_ENTITY_TYPE, table);
}