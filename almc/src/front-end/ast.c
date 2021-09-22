#include "ast.h"

Expr* expr_new(ExprType type, void* expr_value_ptr)
{
	#define expr_set_value(type, field) e->field = (type*)expr_value_ptr
	Expr* e = new_s(Expr, e);
	switch (e->type = type)
	{
	case EXPR_IDNT:
		expr_set_value(Idnt, idnt);
		break;
	case EXPR_CONST:
		expr_set_value(Const, cnst);
		break;
	case EXPR_STRING:
		expr_set_value(Str, str);
		break;
	case EXPR_FUNC_CALL:
		expr_set_value(FuncCall, func_call);
		break;
	case EXPR_UNARY_EXPR:
		expr_set_value(UnaryExpr, unary_expr);
		break;
	case EXPR_BINARY_EXPR:
		expr_set_value(BinaryExpr, binary_expr);
		break;
	case EXPR_TERNARY_EXPR:
		expr_set_value(TernaryExpr, ternary_expr);
		break;
	default:
		assert(0);
	}
	return e;
}

Str* str_new(const char* string, SrcContext* context)
{
	Str* s = new_s(Str, s);
	s->svalue = string;
	s->context = context;
	return s;
}

Idnt* idnt_new(const char* idnt, SrcContext* context)
{
	Idnt* i = new_s(Idnt, i);
	i->svalue = idnt;
	i->context = context;
	return i;
}

Const* const_new(ConstType type, double value, SrcContext* context)
{
	Const* c = new_s(Const, c);
	c->context = context;
	switch (c->type = type)
	{
	case CONST_INT:
		c->ivalue = (int64_t)value;
		break;
	case CONST_UINT:
		c->uvalue = (uint64_t)value;
		break;
	case CONST_FLOAT:
		c->fvalue = value;
		break;
	default:
		assert(0);
	}
	return c;
}

FuncCall* func_call_new(const char* func_name, Expr** func_args)
{
	FuncCall* fc = new_s(FuncCall, fc);
	fc->func_args = func_args;
	fc->func_name = func_name;
	return fc;
}

UnaryExpr* unary_expr_new(UnaryExprType type, Expr* expr)
{
	UnaryExpr* ue = new_s(UnaryExpr, ue);
	ue->type = type;
	ue->expr = expr;
	ue->cast_type = NULL;
	return ue;
}

BinaryExpr* binary_expr_new(BinaryExprType type, Expr* lexpr, Expr* rexpr)
{
	BinaryExpr* be = new_s(BinaryExpr, be);
	be->type = type;
	be->lexpr = lexpr;
	be->rexpr = rexpr;
	return be;
}

TernaryExpr* ternary_expr_new(Expr* cond, Expr* lexpr, Expr* rexpr)
{
	TernaryExpr* te = new_s(TernaryExpr, te);
	te->cond = cond;
	te->lexpr = lexpr;
	te->rexpr = rexpr;
	return te;
}

Stmt* stmt_new(StmtType type, void* stmt_value_ptr)
{
	#define stmt_set_value(type, field) s->field = (type*)stmt_value_ptr
	Stmt* s = new_s(Stmt, s);
	switch (s->type = type)
	{
	case STMT_TYPE_DECL:
		stmt_set_value(TypeDecl, type_decl);
		break;
	case STMT_VAR_DECL:
		stmt_set_value(VarDecl, var_decl);
		break;
	default:
		assert(0);
	}
	return s;
}

TypeDecl* type_decl_new(TypeDeclType type, void* type_decl_value_ptr)
{
	#define type_decl_set_value(type, field) td->field = (type*)type_decl_value_ptr
	TypeDecl* td = new_s(TypeDecl, td);
	switch (td->type = type)
	{
	case TYPE_DECL_ENUM:
		type_decl_set_value(EnumDecl, enum_decl);
		break;
	case TYPE_DECL_UNION:
		type_decl_set_value(UnionDecl, enum_decl);
		break;
	case TYPE_DECL_STRUCT:
		type_decl_set_value(StructDecl, enum_decl);
		break;
	default:
		assert(0);
	}
	return td;
}

EnumDecl* enum_decl_new(Idnt** enum_idnts, Expr** enum_idnt_values, const char* enum_name)
{
	EnumDecl* ed = new_s(EnumDecl, ed);
	ed->enum_name = enum_name;
	ed->enum_idnts = enum_idnts;
	ed->enum_idnt_values = enum_idnt_values;
	return ed;
}

UnionDecl* union_decl_new(VarDecl** union_mmbrs, const char* union_name)
{
	UnionDecl* ud = new_s(UnionDecl, ud);
	ud->union_name = union_name;
	ud->union_mmbrs = union_mmbrs;
	return ud;
}

StructDecl* struct_decl_new(VarDecl** struct_mmbrs, const char* struct_name)
{
	StructDecl* sd = new_s(StructDecl, sd);
	sd->struct_name = struct_name;
	sd->struct_mmbrs = struct_mmbrs;
	return sd;
}

VarDecl* var_decl_new(Type* var_type, Expr* var_init, const char* var_name)
{
	VarDecl* vd = new_s(VarDecl, vd);
	vd->var_type = var_type;
	vd->var_name = var_name;
	vd->var_init = var_init;
	return vd;
}

void print_ast(AstRoot* ast)
{
	printf("ast-root:\n");
	for (uint32_t i = 0; i < sbuffer_len(ast->exprs); i++)
		print_expr(ast->exprs[i], "");
}

void print_expr(Expr* expr, const char* indent)
{
	// |——
	char* new_indent = frmt("%s   ", indent);
	if (expr)
		switch (expr->type)
		{
		case EXPR_IDNT:
			print_idnt(expr->idnt, new_indent);
			break;
		case EXPR_CONST:
			print_const(expr->cnst, new_indent);
			break;
		case EXPR_STRING:
			print_str(expr->str, new_indent);
			break;
		case EXPR_FUNC_CALL:
			print_func_call(expr->func_call, new_indent);
			break;
		case EXPR_UNARY_EXPR:
			print_unary_expr(expr->unary_expr, new_indent);
			break;
		case EXPR_BINARY_EXPR:
			print_binary_expr(expr->binary_expr, new_indent);
			break;
		case EXPR_TERNARY_EXPR:
			print_ternary_expr(expr->ternary_expr, new_indent);
			break;
		}
	else
		printf("%s   no-body\n", indent);
}

void print_type(Type* type, const char* indent)
{
#define print_type_mode(mode) type->mods.mode ? \
	printf("%s%s: %d\n", frmt("%s   ", indent), #mode, type->mods.mode) : 0

	printf(BOLDRED);
	printf("%stype: %s\n", indent, type->repr);
	printf(RESET);
	print_type_mode(is_ptr);
	print_type_mode(is_void);
	print_type_mode(is_static);
	print_type_mode(is_register);
	print_type_mode(is_volatile);
	print_type_mode(is_const_ptr);
	print_type_mode(is_predefined);
}

void print_str(Str* str, const char* indent)
{
	printf(WHITE);
	printf("%sstr: \"%s\"\n", indent, str->svalue);
	printf(RESET);
}

void print_idnt(Idnt* idnt, const char* indent)
{
	printf(BOLDWHITE);
	printf("%sidnt: %s\n", indent, idnt->svalue);
	printf(RESET);
}

void print_const(Const* cnst, const char* indent)
{
	printf(BOLDWHITE);
	switch (cnst->type)
	{
	case CONST_INT:
		printf("%sint-const: %lld\n", indent, cnst->ivalue);
		break;
	case CONST_UINT:
		printf("%suint-const: %I64u\n", indent, cnst->uvalue);
		break;
	case CONST_FLOAT:
		printf("%sfloat-const: %f\n", indent, cnst->fvalue);
		break;
	}
	printf(RESET);
}

void print_func_call(FuncCall* func_call, const char* indent)
{
	printf(GREEN);
	size_t len = sbuffer_len(func_call->func_args);
	printf("%sfunc-call: %s(args: %d)\n", indent,
		func_call->func_name, len);
	printf(RESET);
	if (len)
		for (int i = 0; i < len; i++)
			print_expr(func_call->func_args[i], indent);
	else
		printf("%s   no-args\n", indent);
}

void print_unary_expr(UnaryExpr* expr, const char* indent)
{
	printf(YELLOW);
	const char* unary_expr_type_str[] = {
		"unary-plus: +",
		"unary-minus: -",
		"unary-addr: &",
		"unary-deref: *",

		"unary-lg-not: !",
		"unary-bw-not: ~",

		"unary-inc: ++",
		"unary-dec: --",

		"unary-cast:",
		"unary-sizeof:",

		"unary-postfix-inc: ++",
		"unary-postfix-dec: --",
	};
	switch (expr->type)
	{
	case UNARY_CAST:
	case UNARY_SIZEOF:
		printf("%s%s\n", indent, unary_expr_type_str[expr->type]);
		if (expr->cast_type)
			print_type(expr->cast_type, frmt("%s   ", indent));
		break;
	default:
		printf("%s%s\n", indent, unary_expr_type_str[expr->type]);
	}
	printf(RESET);
	print_expr(expr->expr, indent);
}

void print_binary_expr(BinaryExpr* expr, const char* indent)
{
	printf(BOLDYELLOW);
	const char* binary_expr_type_str[] = {
		"binary-add: +",
		"binary-sub: -",
		"binary-div: /",
		"binary-mod: %",
		"binary-mult: *",

		"binary-shl: <<",
		"binary-shr: >>",
		
		"binary-less-than: <",
		"binary-greater-than: >",
		"binary-less-eq-than: <=",
		"binary-greater-eq-than: >=",

		"binary-lg-or: ||",
		"binary-lg-and: &&",
		"binary-lg-eq: ==",
		"binary-lg-neq: !=",

		"binary-bw-or: |",
		"binary-bw-and: &",
		"binary-bw-xor: ^",

		"binary-asgn: =",
		"binary-add-asgn: +=",
		"binary-sub-asgn: -=",
		"binary-mul-asgn: *=",
		"binary-div-asgn: /=",
		"binary-mod-asgn: %=",
		"binary-shl-asgn: <<=",
		"binary-shr-asgn: >>=",
		"binary-bw-or-asgn: |=",
		"binary-bw-and-asgn: &=",
		"binary-bw-xor-asgn: ^=",
		"binary-bw-not-asgn: ~=",

		"binary-mmbr-accsr: .",
		"binary-ptr-mmbr-accsr: ->",
		"binary-arr-mmbr-accsr: []"
	};
	printf("%s%s\n", indent, binary_expr_type_str[expr->type]);
	printf(RESET);
	print_expr(expr->lexpr, indent);
	print_expr(expr->rexpr, indent);
}

void print_ternary_expr(TernaryExpr* expr, const char* indent)
{
	printf(BOLDGREEN);
	printf("%sternary:\n", indent);
	printf(RESET);
	print_expr(expr->cond, indent);
	print_expr(expr->lexpr, indent);
	print_expr(expr->rexpr, indent);
}

void print_stmt(Stmt* stmt, const char* indent)
{
	char* new_indent = frmt("%s   ", indent);
	if (stmt)
		switch (stmt->type)
		{
		case STMT_VAR_DECL:
			print_var_decl(stmt->var_decl, new_indent);
			break;
		case STMT_TYPE_DECL:
			print_type_decl(stmt->type_decl, new_indent);
			break;
		default:
			assert(0);
		}
	else
		printf("%s   no-body\n", indent);
}

void print_type_decl(TypeDecl* type_decl, const char* indent)
{
	switch (type_decl->type)
	{
	case TYPE_DECL_ENUM:
		print_enum_decl(type_decl->enum_decl, indent);
		break;
	case TYPE_DECL_STRUCT:
		print_struct_decl(type_decl->struct_decl, indent);
		break;
	default:
		assert(0);
	}
}

void print_enum_decl(EnumDecl* enum_decl, const char* indent)
{
	printf(BOLDCYAN);
	printf("%senum-decl: %s {idnts: %d}\n", indent, enum_decl->enum_name, sbuffer_len(enum_decl->enum_idnts));
	printf(RESET);
	for (int i = 0; i < sbuffer_len(enum_decl->enum_idnts); i++)
	{
		printf("%s   %s:\n", indent, enum_decl->enum_idnts[i]->svalue);
		print_expr(enum_decl->enum_idnt_values[i], frmt("   %s", indent));
	}
}

void print_union_decl(UnionDecl* union_decl, const char* indent)
{
	assert(0);
}

void print_struct_decl(StructDecl* struct_decl, const char* indent)
{
	printf(BOLDCYAN);
	printf("%sstruct-decl: %s {idnts: %d}\n", indent, struct_decl->struct_name, sbuffer_len(struct_decl->struct_mmbrs));
	printf(RESET);
	for (int i = 0; i < sbuffer_len(struct_decl->struct_mmbrs); i++)
		print_var_decl(struct_decl->struct_mmbrs[i], frmt("   %s", indent));
}

void print_var_decl(VarDecl* var_decl, const char* indent)
{
	printf(CYAN);
	printf("%svar-decl: %s\n", indent, var_decl->var_name);
	printf(RESET);
	indent = frmt("   %s", indent);
	print_type(var_decl->var_type, indent);
	printf("%sinit-expr:\n", indent);
	print_expr(var_decl->var_init, indent);
}