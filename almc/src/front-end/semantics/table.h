#ifndef _ALMC_TABLE2
#define _ALMC_TABLE2

#include "..\ast\ast.h"
#include "..\..\utils\common.h"
#include "..\..\utils\data-structures\sbuffer.h"

typedef enum TableEntityKind
{
	TABLE_ENTITY_ENUM,
	TABLE_ENTITY_UNION,
	TABLE_ENTITY_STRUCT,
	TABLE_ENTITY_VARIABLE,
	TABLE_ENTITY_PARAMETER,
	TABLE_ENTITY_ENUM_MEMBER,

	TABLE_ENTITY_LABEL,
	TABLE_ENTITY_FUNCTION
} TableEntityKind;

typedef struct TableEntity
{
	bool is_in_use;
	bool is_unresolved;

	//todo: add logic for used/not used entity

	union _table_entity_value
	{
		VarDecl* local;
		LabelDecl* label;
		TypeVar* parameter;
		FuncDecl* function;
		EnumMember* enum_member;

		EnumDecl* enum_decl;
		UnionDecl* union_decl;
		StructDecl* struct_decl;
	};

	struct _table_entity_variable_data
	{
		bool is_ref_to_func;
		bool is_initialized;
	};

	enum TableEntityKind kind;
} TableEntity;

typedef struct Table
{
	struct _scope_data
	{
		TableEntity** locals;
		TableEntity** labels;
		TableEntity** functions;
		TableEntity** parameters;
		TableEntity** enum_members;

		struct _scope_typedecls_data
		{
			TableEntity** enums;
			TableEntity** unions;
			TableEntity** structs;
		};
	};

	struct _scope_references
	{
		LoopStmt*   in_loop;
		SwitchStmt* in_switch;

		FuncDecl* in_function;
	};

	struct Table*  nested_in;
	struct Table** nesting;
} Table;

Table* table_new(Table* parent);
void table_free(Table* table);

TableEntity* table_entity_new(TableEntityKind kind);
void table_entity_free(TableEntity* table_entity);
void table_entities_free(TableEntity** entities);

bool is_function_declared(const char* func_name, Table* table);
bool is_variable_declared(const char* var_name, Table* table);
bool is_function_param_passed(const char* param_name, Table* table);
bool is_label_declared(const char* label_name, Table* table);
bool is_variable_initialized(const char* var_name, Table* table);
bool is_enum_declared(const char* enum_name, Table* table);
bool is_struct_declared(const char* struct_name, Table* table);
bool is_union_declared(const char* union_name, Table* table);

bool is_table_entity_declared(const char* decl_name,
	TableEntityKind kind, Table* table);
bool add_table_entity(TableEntity*** entities, void* decl,
	const char* decl_name, TableEntityKind kind, Table* table);

bool add_func2(FuncDecl* func_decl, Table* table);
void add_variable(VarDecl* var_decl, Table* table);
bool add_variable2(VarDecl* var_decl, Table* table);
bool add_label2(LabelDecl* label_decl, Table* table);
bool add_parameter(TypeVar* type_var, Table* table);
void add_initialized_variable(char* var_name, Table* table);
bool add_enum_member(EnumMember* enum_member, Table* table);
bool add_enum2(EnumDecl* enum_decl, Table* table);
bool add_struct2(StructDecl* struct_decl, Table* table);
bool add_union2(UnionDecl* union_decl, Table* table);

TableEntity* get_table_entity(const char* entity_name, 
	TableEntityKind kind, Table* table);

TableEntity* get_enum_member(const char* enum_member_name, Table* table);
TableEntity* get_parameter(const char* parameter_name, Table* table);
TableEntity* get_variable(const char* var_name, Table* table);

TableEntity* get_label(const char* label_name, Table* table);
TableEntity* get_function(const char* func_name, Table* table);
TableEntity* get_function_param(const char* param_name, Table* table);
TableEntity* get_enum(const char* enum_name, Table* table);
TableEntity* get_union(const char* union_name, Table* table);
TableEntity* get_struct(const char* struct_name, Table* table);

#endif // _ALMC_TABLE2