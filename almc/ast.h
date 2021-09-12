#ifndef ALMC_AST_H
#define ALMC_AST_H

//todo: how to deal with ast printing?
//TODO: use c standart for grammar

//todo: add custom keywords
//todo: for now add simple arithmetic parsing

#include "common.h"
#include "context.h"
#include "sbuffer.h"

typedef struct Expr Expr;

typedef enum UnaryExprType
{
	UNARY_PLUS,
	UNARY_MINUS,
	UNARY_LG_NOT,
	UNARY_BW_NOT,

	UNARY_CAST,
	UNARY_SIZEOF,
} UnaryExprType;

typedef struct UnaryExpr
{
	Expr* expr;
	char* cast_type;
	UnaryExprType type;
} UnaryExpr;

typedef enum BinaryExprType
{
	BINARY_ADD,
	BINARY_SUB,
	BINARY_DIV,
	BINARY_MOD,
	BINARY_MULT,

	BINARY_LG_OR,
	BINARY_LG_AND,
	BINARY_LG_EQ,
	BINARY_LG_NEQ,

	BINARY_BW_OR,
	BINARY_BW_AND,
	BINARY_BW_XOR,
} BinaryExprType;

typedef struct BinaryExpr
{
	Expr* lexpr;
	Expr* rexpr;
	BinaryExprType type;
} BinaryExpr;

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

typedef enum ExprType
{
	EXPR_IDNT,
	EXPR_CONST,
	EXPR_UNARY_EXPR,
	EXPR_BINARY_EXPR,
} ExprType;

typedef struct Expr
{
	ExprType type;
	union
	{
		Idnt* idnt;
		Const* cnst;
		UnaryExpr* unary_expr;
		BinaryExpr* binary_expr;
	};
} Expr;

typedef struct AstRoot
{
	Expr** exprs;
} AstRoot;

Expr* expr_new(ExprType type, void* expr_value_ptr);
Idnt* idnt_new(const char* idnt, SrcContext* context);
Const* const_new(ConstType type, double value, SrcContext* context);
UnaryExpr* unary_expr_new(UnaryExprType type, Expr* expr);
BinaryExpr* binary_expr_new(BinaryExprType type, Expr* lexpr, Expr* rexpr);

void print_ast(AstRoot* ast);
void print_expr(Expr* expr, const char* indent);
void print_idnt(Idnt* idnt, const char* indent);
void print_const(Const* cnst, const char* indent);
void print_unary_expr(UnaryExpr* expr, const char* indent);
void print_binary_expr(BinaryExpr* expr, const char* indent);

/*typedef enum PrimaryExprType
{
	TYPE_EXPR,
	TYPE_CONSTANT,
	TYPE_IDENTIFIER,
	TYPE_STR_LITERAL,
} PrimaryExprType;

typedef enum ConstantType
{
	CONST_INUM,
	CONST_FNUM,
	CONST_UNUM,
} ConstantType;

typedef struct Identifier
{
	char is_type;
	SrcContext context;
	const char* svalue;
} Identifier;

typedef struct Constant
{
	SrcContext context;
	ConstantType type;
	union
	{
		double fvalue;
		int64_t ivalue;
		uint64_t uvalue;
	};
} Constant;

typedef struct StrLiteral
{
	SrcContext context;
	const char* svalue;
} StrLiteral;

typedef struct Expr
{
	union
	{
		int a;
		//...
	};
} Expr;

typedef struct PrimaryExpr
{
	PrimaryExprType type;
	union
	{
		Expr* expr;
		Constant* cnst;
		Identifier* idnt;
		StrLiteral* sliteral;
	};
} PrimaryExpr;


Identifier* idnt_new(const char* idnt, SrcContext* context);
StrLiteral* str_literal_new(const char* str, SrcContext* context);
Constant* const_new(ConstantType type, void* value_ptr, SrcContext* context);

Expr* expr_new();
PrimaryExpr* pexpr_new();

void set_value_of_const(ConstantType type, void* value_ptr);
void set_value_of_pexpr(PrimaryExprType type, void* value_ptr);*/

#endif // AST_H 