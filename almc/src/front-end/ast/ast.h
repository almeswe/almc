#ifndef ALMC_AST_H
#define ALMC_AST_H

#include "..\type.h"
#include "..\lexer.h"

typedef struct Expr Expr;
typedef struct Stmt Stmt;
typedef struct AstRoot AstRoot;

typedef enum UnaryExprKind
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
	UNARY_LENGTHOF,

	UNARY_POSTFIX_INC,
	UNARY_POSTFIX_DEC,
} UnaryExprKind;

typedef struct UnaryExpr
{
	Type* type;
	Expr* expr;
	Type* cast_type;
	SrcArea* area;
	UnaryExprKind kind;
} UnaryExpr;

typedef enum BinaryExprKind
{
	BINARY_ADD,
	BINARY_SUB,
	BINARY_DIV,
	BINARY_MOD,
	BINARY_MULT,

	BINARY_COMMA,
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
} BinaryExprKind;

typedef struct BinaryExpr
{
	Type* type;
	Expr* lexpr;
	Expr* rexpr;
	SrcArea* area;
	BinaryExprKind kind;
} BinaryExpr;

typedef struct TernaryExpr
{
	Type* type;
	Expr* cond;
	Expr* lexpr;
	Expr* rexpr;
	SrcArea* area;
} TernaryExpr;

typedef struct Str
{
	Type* type;
	const char* svalue;
	SrcContext* context;
} Str;

typedef struct Idnt
{
	Type* type;
	const char* svalue;
	SrcContext* context;
} Idnt;

typedef enum ConstKind
{
	CONST_INT,
	CONST_UINT,
	CONST_FLOAT,
	CONST_CHAR,
} ConstKind;

typedef struct Const
{
	Type* type;
	ConstKind kind;
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
	Type* type;
	Expr** args;
	SrcArea* area;
	const char* name;
} FuncCall;

typedef struct Initializer
{
	Type* type;
	Expr** values;
	SrcArea* area;
} Initializer;

typedef enum ExprKind
{
	EXPR_IDNT,
	EXPR_CONST,
	EXPR_STRING,
	EXPR_FUNC_CALL,
	EXPR_UNARY_EXPR,
	EXPR_BINARY_EXPR,
	EXPR_TERNARY_EXPR,
	EXPR_INITIALIZER,
} ExprKind;

typedef struct Expr
{
	ExprKind kind;
	union
	{
		Str* str;
		Idnt* idnt;
		Const* cnst;
		FuncCall* func_call;
		UnaryExpr* unary_expr;
		BinaryExpr* binary_expr;
		TernaryExpr* ternary_expr;
		Initializer* initializer;
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
	SrcArea* area;
	const char* var;
} TypeVar;

typedef struct VarDecl
{
	Expr* var_init;
	TypeVar* type_var;
} VarDecl;

typedef struct FuncSpecifiers
{
	bool is_entry;
	bool is_intrinsic;
} FuncSpecifiers;

typedef struct FuncDecl
{
	Idnt* name;
	Type* type;
	Block* body;
	TypeVar** params;
	FuncSpecifiers spec;
} FuncDecl;

typedef struct LabelDecl
{
	Idnt* label;
} LabelDecl;

typedef struct EnumMember
{
	char* name;
	Expr* value;
	SrcContext* context;
} EnumMember;

typedef struct EnumDecl
{
	char* name;
	EnumMember** members;
} EnumDecl;

typedef struct Member
{
	char* name;
	Type* type;
	SrcArea* area;
	int32_t offset;
	int32_t padding;
} Member;

typedef struct UnionDecl
{
	char* name;
	Member** members;
} UnionDecl;

typedef struct StructDecl
{
	char* name;
	Member** members;
	uint32_t alignment;
} StructDecl;

typedef enum TypeDeclKind
{
	TYPE_DECL_ENUM,
	TYPE_DECL_UNION,
	TYPE_DECL_STRUCT,
} TypeDeclKind;

typedef struct TypeDecl
{
	TypeDeclKind kind;
	union
	{
		EnumDecl* enum_decl;
		UnionDecl* union_decl;
		StructDecl* struct_decl;
	};
} TypeDecl;

typedef struct DoLoop
{
	Expr* cond;
	Block* body;
} DoLoop;

typedef struct ForLoop
{
	Expr* cond;
	Expr* step;
	Block* body;
	VarDecl* init;
} ForLoop;

typedef struct WhileLoop
{
	Expr* cond;
	Block* body;
} WhileLoop;

typedef enum LoopStmtKind
{
	LOOP_DO,
	LOOP_FOR,
	LOOP_WHILE,
} LoopStmtKind;

typedef struct LoopStmt
{
	LoopStmtKind kind;
	union
	{
		DoLoop* do_loop;
		ForLoop* for_loop;
		WhileLoop* while_loop;
	};
} LoopStmt;

typedef struct ElseIf
{
	Expr* cond;
	Block* body;
} ElseIf;

typedef struct IfStmt
{
	Expr* cond;
	Block* body;
	Block* else_body;

	ElseIf** elifs;
} IfStmt;

typedef struct Case
{
	Expr* value;
	Block* body;
	// means that this case statement has the same body with next case
	bool is_conjucted;
} Case;

typedef struct SwitchStmt
{
	Expr* cond;
	Case** cases;
	Block* default_case;
} SwitchStmt;

typedef struct EmptyStmt
{
	char filler[0];
} EmptyStmt;

typedef enum JumpStmtKind
{
	JUMP_GOTO,
	JUMP_BREAK,
	JUMP_RETURN,
	JUMP_CONTINUE,
} JumpStmtKind;

typedef struct JumpStmt
{
	SrcArea* area;
	JumpStmtKind kind;
	//used by goto && return stmts
	Expr* additional_expr;
} JumpStmt;

typedef struct ImportStmt
{
	AstRoot* ast;
} ImportStmt;

typedef enum StmtType
{
	STMT_IF,
	STMT_EXPR,
	STMT_LOOP,
	STMT_JUMP,
	STMT_BLOCK,
	STMT_EMPTY,
	STMT_SWITCH,
	STMT_IMPORT,
	STMT_VAR_DECL,
	STMT_TYPE_DECL,
	STMT_FUNC_DECL,
	STMT_LABEL_DECL,
} StmtType;

typedef struct Stmt
{
	StmtType kind;
	union
	{
		Block* block;
		IfStmt* if_stmt;
		LoopStmt* loop_stmt;
		ExprStmt* expr_stmt;
		JumpStmt* jump_stmt;
		EmptyStmt* empty_stmt;
		SwitchStmt* switch_stmt;
		ImportStmt* import_stmt;

		VarDecl* var_decl;
		TypeDecl* type_decl;
		FuncDecl* func_decl;
		LabelDecl* label_decl;
	};
} Stmt;

typedef struct AstRoot
{
	Stmt** stmts;
} AstRoot;

Expr* expr_new(ExprKind type, void* expr_value_ptr);

Str* str_new(const char* string, SrcContext* context);
Idnt* idnt_new(const char* idnt, SrcContext* context);
Const* const_new(ConstKind type, const char* svalue, SrcContext* context);
FuncCall* func_call_new(const char* name, Expr** args);
UnaryExpr* unary_expr_new(UnaryExprKind type, Expr* expr);
BinaryExpr* binary_expr_new(BinaryExprKind type, Expr* lexpr, Expr* rexpr);
TernaryExpr* ternary_expr_new(Expr* cond, Expr* lexpr, Expr* rexpr);
Initializer* initializer_new(Expr** values);
TypeVar* type_var_new(Type* type, const char* var);

Stmt* stmt_new(StmtType type, void* stmt_value_ptr);

Block* block_new(Stmt** stmts);
ElseIf* elif_stmt_new(Expr* cond, Block* body);
IfStmt* if_stmt_new(Expr* cond, Block* body, ElseIf** elifs, Block* else_body);

DoLoop* do_loop_new(Expr* cond, Block* body);
ForLoop* for_loop_new(VarDecl* init, Expr* cond, Expr* step, Block* body);
WhileLoop* while_loop_new(Expr* cond, Block* body);
LoopStmt* loop_stmt_new(LoopStmtKind type, void* loop_stmt_value_ptr);

ExprStmt* expr_stmt_new(Expr* expr);

EmptyStmt* empty_stmt_new();
JumpStmt* jump_stmt_new(JumpStmtKind type, Expr* return_expr);
ImportStmt* import_stmt_new(AstRoot* ast);

Case* case_stmt_new(Expr* value, Block* body, bool is_conjucted);
SwitchStmt* switch_stmt_new(Expr* cond, Case** cases, Block* default_case);

LabelDecl* label_decl_new(Idnt* label);
VarDecl* var_decl_new(TypeVar* type_var, Expr* init);
FuncDecl* func_decl_new(Idnt* name, TypeVar** params, Type* type, Block* body, FuncSpecifiers spec);

TypeDecl* type_decl_new(TypeDeclKind type, void* type_decl_value_ptr);
EnumDecl* enum_decl_new(EnumMember** members, const char* name);
UnionDecl* union_decl_new(Member** members, const char* name);
StructDecl* struct_decl_new(Member** members, const char* name);

Member* member_new(char* name, Type* type, SrcArea* area);
EnumMember* enum_member_new(char* name, Expr* value, SrcContext* context);

void ast_free(AstRoot* root);

void expr_free(Expr* expr);
void str_free(Str* str);
void idnt_free(Idnt* idnt);
void const_free(Const* cnst);
void func_call_free(FuncCall* func_call);
void unary_expr_free(UnaryExpr* unary_expr);
void binary_expr_free(BinaryExpr* binary_expr);
void ternary_expr_free(TernaryExpr* ternary_expr);
void initializer_free(Initializer* initializer);

void stmt_free(Stmt* stmt);
void type_decl_free(TypeDecl* type_decl);
void enum_decl_free(EnumDecl* enum_decl);
void union_decl_free(UnionDecl* union_decl);
void struct_decl_free(StructDecl* struct_decl);
void member_free(Member* member);

void empty_stmt_free(EmptyStmt* empty_stmt);
void expr_stmt_free(ExprStmt* expr_stmt);

void block_free(Block* block);
void type_var_free(TypeVar* type_var);
void var_decl_free(VarDecl* var_decl);
void func_decl_free(FuncDecl* func_decl);
void label_decl_free(LabelDecl* label_decl);

void loop_stmt_free(LoopStmt* loop_stmt);
void do_loop_free(DoLoop* do_loop);
void for_loop_free(ForLoop* for_loop);
void while_loop_free(WhileLoop* while_loop);

void elif_stmt_free(ElseIf* elif_stmt);
void if_stmt_free(IfStmt* if_stmt);

void case_stmt_free(Case* case_stmt);
void switch_stmt_free(SwitchStmt* switch_stmt);
void import_stmt_free(ImportStmt* import_stmt);

void jump_stmt_free(JumpStmt* jump_stmt);

#endif