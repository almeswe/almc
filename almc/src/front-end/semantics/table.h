#ifndef ALMC_FRONT_END_TABLE_H
#define ALMC_FRONT_END_TABLE_H

#include "..\ast\ast.h"
#include "..\..\utils\common.h"
#include "..\..\utils\data-structures\sbuffer.h"

typedef struct Table
{
	VarDecl** variables;
	TypeVar** parameters;
	FuncDecl** functions;

	//-------------------
	// type definitions
	EnumDecl** enums;
	UnionDecl** unions;
	StructDecl** structs;
	//-------------------

	//-------------------
	// few extensions added to avoid parent passing to each node of ast

	// current function in which this table declared
	// needeed mostly for return statement
	FuncDecl* in_function;

	// last loop | switch mentioned in this scope
	// needed for continue, break stmts
	LoopStmt* in_loop;
	SwitchStmt* in_switch;
	//-------------------

	char** initialized_variables_in_scope;

	// included local scopes of this scope
	struct Table* parent;
	struct Table** childs;
} Table;

Table* table_new(Table* parent);
void table_free(Table* table);

int is_function_declared(const char* func_name, Table* table);
int is_variable_declared(const char* var_name, Table* table);
int is_function_param_passed(const char* param_name, Table* table);
int is_variable_initialized(const char* var_name, Table* table);
int is_enum_declared(const char* enum_name, Table* table);
int is_struct_declared(const char* struct_name, Table* table);
int is_union_declared(const char* union_name, Table* table);

void add_function(FuncDecl* func_decl, Table* table);
void add_variable(VarDecl* var_decl, Table* table);
void add_function_param(TypeVar* type_var, Table* table);
void add_initialized_variable(char* var_name, Table* table);
void add_enum(EnumDecl* enum_decl, Table* table);
void add_struct(StructDecl* struct_decl, Table* table);
void add_union(UnionDecl* union_decl, Table* table);

VarDecl* get_variable(const char* var_name, Table* table);
FuncDecl* get_function(const char* func_name, Table* table);
TypeVar* get_function_param(const char* param_name, Table* table);
EnumDecl* get_enum(const char* enum_name, Table* table);
UnionDecl* get_union(const char* union_name, Table* table);
StructDecl* get_struct(const char* struct_name, Table* table);

#endif