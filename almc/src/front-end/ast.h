#ifndef ALMC_AST_H
#define ALMC_AST_H

#include "lexer.h"
#include "..\utils\common.h"
#include "..\utils\context.h"
#include "..\utils\sbuffer.h"

typedef struct Expr Expr;
typedef struct Stmt Stmt;

typedef struct TypeMods
{
	char is_ptr; // 0 - not pointer type, > 0 pointer type + pointer counter
	char is_void;
	char is_const;
	char is_static;
	char is_register;
	char is_volatile;
	char is_const_ptr;
	char is_predefined;
} TypeMods;

typedef struct Type
{
	TypeMods mods;
	const char* repr;
} Type;

typedef enum UnaryExprType
{
	UNARY_PLUS,
	UNARY_MINUS,
	UNARY_ADDRESS,
	UNARY_DEREFERENCE,

	UNARY_LG_NOT,
	UNARY_BW_NOT,

	UNARY_PREFIX_INC,
	UNARY_PREFIX_DEC,

	UNARY_CAST,
	UNARY_SIZEOF,

	UNARY_POSTFIX_INC,
	UNARY_POSTFIX_DEC,
} UnaryExprType;

typedef struct UnaryExpr
{
	Expr* expr;
	Type* cast_type;
	UnaryExprType type;
} UnaryExpr;

typedef enum BinaryExprType
{
	BINARY_ADD,
	BINARY_SUB,
	BINARY_DIV,
	BINARY_MOD,
	BINARY_MULT,

	BINARY_LSHIFT,
	BINARY_RSHIFT,

	BINARY_LESS_THAN,
	BINARY_GREATER_THAN,
	BINARY_LESS_EQ_THAN,
	BINARY_GREATER_EQ_THAN,

	BINARY_LG_OR,
	BINARY_LG_AND,
	BINARY_LG_EQ,
	BINARY_LG_NEQ,

	BINARY_BW_OR,
	BINARY_BW_AND,
	BINARY_BW_XOR,

	BINARY_ASSIGN,
	BINARY_ADD_ASSIGN,
	BINARY_SUB_ASSIGN,
	BINARY_MUL_ASSIGN,
	BINARY_DIV_ASSIGN,
	BINARY_MOD_ASSIGN,
	BINARY_LSHIFT_ASSIGN,
	BINARY_RSHIFT_ASSIGN,

	BINARY_BW_OR_ASSIGN,
	BINARY_BW_AND_ASSIGN,
	BINARY_BW_XOR_ASSIGN,
	BINARY_BW_NOT_ASSIGN,

	BINARY_MEMBER_ACCESSOR,
	BINARY_PTR_MEMBER_ACCESSOR,
	BINARY_ARR_MEMBER_ACCESSOR,
} BinaryExprType;

typedef struct BinaryExpr
{
	Expr* lexpr;
	Expr* rexpr;
	BinaryExprType type;
} BinaryExpr;

typedef struct TernaryExpr
{
	Expr* cond;
	Expr* lexpr;
	Expr* rexpr;
} TernaryExpr;

typedef struct Str
{
	const char* svalue;
	SrcContext* context;
} Str;

typedef struct Idnt
{
	const char* svalue;
	SrcContext* context;
} Idnt;

typedef enum ConstType
{
	CONST_INT,
	CONST_UINT,
	CONST_FLOAT,
} ConstType;

typedef struct Const
{
	ConstType type;
	union
	{
		double fvalue;
		int64_t ivalue;
		uint64_t uvalue;
	};
	SrcContext* context;
} Const;

typedef struct FuncCall
{
	Expr** func_args;
	const char* func_name;
} FuncCall;

typedef enum ExprType
{
	EXPR_IDNT,
	EXPR_CONST,
	EXPR_STRING,
	EXPR_FUNC_CALL,
	EXPR_UNARY_EXPR,
	EXPR_BINARY_EXPR,
	EXPR_TERNARY_EXPR,
} ExprType;

typedef struct Expr
{
	ExprType type;
	union
	{
		Str* str;
		Idnt* idnt;
		Const* cnst;
		FuncCall* func_call;
		UnaryExpr* unary_expr;
		BinaryExpr* binary_expr;
		TernaryExpr* ternary_expr;
	};
} Expr;

typedef struct Block
{
	Stmt** stmts;
} Block;

typedef struct ExprStmt
{
	Expr* expr;
} ExprStmt;

typedef struct TypeVar
{
	Type* type;
	const char* var;
} TypeVar;

typedef struct VarDecl
{
	Expr* var_init;
	TypeVar* type_var;
} VarDecl;

typedef struct FuncDecl
{
	Type* func_type;
	Block* func_body;
	TypeVar** func_params;
	const char* func_name;
} FuncDecl;

typedef struct EnumDecl
{
	Idnt** enum_idnts;
	Expr** enum_idnt_values;
	const char* enum_name;
} EnumDecl;

typedef struct UnionDecl
{
	TypeVar** union_mmbrs;
	const char* union_name;
} UnionDecl;

typedef struct StructDecl
{
	TypeVar** struct_mmbrs;
	const char* struct_name;
} StructDecl;

typedef enum TypeDeclType
{
	TYPE_DECL_ENUM,
	TYPE_DECL_UNION,
	TYPE_DECL_STRUCT,
} TypeDeclType;

typedef struct TypeDecl
{
	TypeDeclType type;
	union
	{
		EnumDecl* enum_decl;
		UnionDecl* union_decl;
		StructDecl* struct_decl;
	};
} TypeDecl;

typedef struct DoLoop
{
	Expr* do_cond;
	Block* do_body;
} DoLoop;

typedef struct ForLoop
{
	Expr* for_cond;
	Expr* for_step;
	Block* for_body;
	VarDecl* for_init;
} ForLoop;

typedef struct WhileLoop
{
	Expr* while_cond;
	Block* while_body;
} WhileLoop;

typedef enum LoopStmtType
{
	LOOP_DO,
	LOOP_FOR,
	LOOP_WHILE,
} LoopStmtType;

typedef struct LoopStmt
{
	LoopStmtType type;
	union
	{
		DoLoop* do_loop;
		ForLoop* for_loop;
		WhileLoop* while_loop;
	};
} LoopStmt;

typedef struct ElseIf
{
	Expr* elif_cond;
	Block* elif_body;
} ElseIf;

typedef struct IfStmt
{
	Expr* if_cond;
	Block* if_body;
	Block* else_body;

	ElseIf** elifs;
} IfStmt;

typedef struct Case
{
	Expr* case_value;
	Block* case_body;
} Case;

typedef struct SwitchStmt
{
	Expr* switch_cond;
	Case** switch_cases;
	Block* switch_default;
} SwitchStmt;

typedef enum StmtType
{	
	STMT_IF,
	STMT_EXPR,
	STMT_LOOP,
	STMT_JUMP,
	STMT_BLOCK,
	STMT_EMPTY,
	STMT_SWITCH,
	STMT_VAR_DECL,
	STMT_TYPE_DECL,
	STMT_FUNC_DECL,
} StmtType;

typedef struct EmptyStmt
{
	char filler[0];
} EmptyStmt;

typedef enum JumpStmtType
{
	JUMP_BREAK,
	JUMP_RETURN,
	JUMP_CONTINUE,
} JumpStmtType;

typedef struct JumpStmt
{
	Expr* return_expr;
	JumpStmtType type;
} JumpStmt;

typedef struct Stmt
{
	StmtType type;
	union
	{
		//todo: add stmt endian ?
		Block* block;
		IfStmt* if_stmt;
		VarDecl* var_decl;
		TypeDecl* type_decl;
		FuncDecl* func_decl;
		LoopStmt* loop_stmt;
		ExprStmt* expr_stmt;
		JumpStmt* jump_stmt;
		EmptyStmt* empty_stmt;
		SwitchStmt* switch_stmt;
	};
} Stmt;

typedef struct AstRoot
{
	Expr** exprs;
} AstRoot;

Expr* expr_new(ExprType type, void* expr_value_ptr);

Str* str_new(const char* string, SrcContext* context);
Idnt* idnt_new(const char* idnt, SrcContext* context);
Const* const_new(ConstType type, double value, SrcContext* context);
FuncCall* func_call_new(const char* func_name, Expr** func_args);
UnaryExpr* unary_expr_new(UnaryExprType type, Expr* expr);
BinaryExpr* binary_expr_new(BinaryExprType type, Expr* lexpr, Expr* rexpr);
TernaryExpr* ternary_expr_new(Expr* cond, Expr* lexpr, Expr* rexpr);

Stmt* stmt_new(StmtType type, void* stmt_value_ptr);
TypeDecl* type_decl_new(TypeDeclType type, void* type_decl_value_ptr);
EnumDecl* enum_decl_new(Idnt** enum_idnts, Expr** enum_idnt_values, const char* enum_name);
UnionDecl* union_decl_new(TypeVar** union_mmbrs, const char* union_name);
StructDecl* struct_decl_new(TypeVar** struct_mmbrs, const char* struct_name);

EmptyStmt* empty_stmt_new();
ExprStmt* expr_stmt_new(Expr* expr);

Block* block_new(Stmt** stmts);
TypeVar* type_var_new(Type* type, const char* var);
VarDecl* var_decl_new(TypeVar* type_var, Expr* var_init);
FuncDecl* func_decl_new(const char* func_name, TypeVar** func_params, Type* func_type, Block* func_body);

LoopStmt* loop_stmt_new(LoopStmtType type, void* loop_stmt_value_ptr);
DoLoop* do_loop_new(Expr* do_cond, Block* do_body);
ForLoop* for_loop_new(VarDecl* for_init, Expr* for_cond, Expr* for_step, Block* for_body);
WhileLoop* while_loop_new(Expr* while_cond, Block* while_body);

ElseIf* elif_stmt_new(Expr* elif_cond, Block* elif_body);
IfStmt* if_stmt_new(Expr* if_cond, Block* if_body, ElseIf** elifs, Block* else_body);

Case* case_stmt_new(Expr* case_value, Block* case_body);
SwitchStmt* switch_stmt_new(Expr* switch_cond, Case** switch_cases, Block* switch_default);

JumpStmt* jump_stmt_new(JumpStmtType type, Expr* return_expr);

void type_free(Type* type);

void expr_free(Expr* expr);
void str_free(Str* str);
void idnt_free(Idnt* idnt);
void const_free(Const* cnst);
void func_call_free(FuncCall* func_call);
void unary_expr_free(UnaryExpr* unary_expr);
void binary_expr_free(BinaryExpr* binary_expr);
void ternary_expr_free(TernaryExpr* ternary_expr);

void stmt_free(Stmt* stmt);
void type_decl_free(TypeDecl* type_decl);
void enum_decl_free(EnumDecl* enum_decl);
void union_decl_free(UnionDecl* union_decl);
void struct_decl_free(StructDecl* struct_decl);

void empty_stmt_free(EmptyStmt* empty_stmt);
void expr_stmt_free(ExprStmt* expr_stmt);

void block_free(Block* block);
void type_var_free(TypeVar* type_var);
void var_decl_free(VarDecl* var_decl);
void func_decl_free(FuncDecl* func_decl);

void loop_stmt_free(LoopStmt* loop_stmt);
void do_loop_free(DoLoop* do_loop);
void for_loop_free(ForLoop* for_loop);
void while_loop_free(WhileLoop* while_loop);

void elif_stmt_free(ElseIf* elif_stmt);
void if_stmt_free(IfStmt* if_stmt);

void case_stmt_free(Case* case_stmt);
void switch_stmt_free(SwitchStmt* switch_stmt);

void jump_stmt_free(JumpStmt* jump_stmt);

#endif // AST_H 