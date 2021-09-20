#ifndef ALMC_AST_H
#define ALMC_AST_H

#include "lexer.h"
#include "..\utils\common.h"
#include "..\utils\context.h"
#include "..\utils\sbuffer.h"


typedef struct Expr Expr;

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

void print_ast(AstRoot* ast);
void print_expr(Expr* expr, const char* indent);
void print_str(Str* str, const char* indent);
void print_idnt(Idnt* idnt, const char* indent);
void print_const(Const* cnst, const char* indent);
void print_func_call(FuncCall* func_call, const char* indent);
void print_unary_expr(UnaryExpr* expr, const char* indent);
void print_binary_expr(BinaryExpr* expr, const char* indent);
void print_ternary_expr(TernaryExpr* expr, const char* indent);

#endif // AST_H 