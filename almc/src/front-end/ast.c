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
	#define stmt_set_value(type, field) s->field = (type*)stmt_value_ptr; break
	Stmt* s = new_s(Stmt, s);
	switch (s->type = type)
	{
	case STMT_IF:
		stmt_set_value(IfStmt, if_stmt);
	case STMT_EXPR:
		stmt_set_value(ExprStmt, expr_stmt);
	case STMT_BLOCK:
		stmt_set_value(Block, block);
	case STMT_LOOP:
		stmt_set_value(LoopStmt, loop_stmt);
	case STMT_EMPTY:
		stmt_set_value(EmptyStmt, empty_stmt);
	case STMT_VAR_DECL:
		stmt_set_value(VarDecl, var_decl);
	case STMT_TYPE_DECL:
		stmt_set_value(TypeDecl, type_decl);
	case STMT_FUNC_DECL:
		stmt_set_value(FuncDecl, func_decl);
	default:
		assert(0);
	}
	return s;
}

TypeDecl* type_decl_new(TypeDeclType type, void* type_decl_value_ptr)
{
	#define type_decl_set_value(type, field) td->field = (type*)type_decl_value_ptr; break
	TypeDecl* td = new_s(TypeDecl, td);
	switch (td->type = type)
	{
	case TYPE_DECL_ENUM:
		type_decl_set_value(EnumDecl, enum_decl);
	case TYPE_DECL_UNION:
		type_decl_set_value(UnionDecl, enum_decl);
	case TYPE_DECL_STRUCT:
		type_decl_set_value(StructDecl, enum_decl);
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

UnionDecl* union_decl_new(TypeVar** union_mmbrs, const char* union_name)
{
	UnionDecl* ud = new_s(UnionDecl, ud);
	ud->union_name = union_name;
	ud->union_mmbrs = union_mmbrs;
	return ud;
}

StructDecl* struct_decl_new(TypeVar** struct_mmbrs, const char* struct_name)
{
	StructDecl* sd = new_s(StructDecl, sd);
	sd->struct_name = struct_name;
	sd->struct_mmbrs = struct_mmbrs;
	return sd;
}

EmptyStmt* empty_stmt_new()
{
	EmptyStmt* es = new_s(EmptyStmt, es);
	return es;
}

ExprStmt* expr_stmt_new(Expr* expr)
{
	ExprStmt* es = new_s(ExprStmt, es);
	es->expr = expr;
	return es;
}

Block* block_new(Stmt** stmts)
{
	Block* b = new_s(Block, b);
	b->stmts = stmts;
	return b;
}

TypeVar* type_var_new(Type* type, const char* var)
{
	TypeVar* tv = new_s(TypeVar, tv);
	tv->var = var;
	tv->type = type;
	return tv;
}

VarDecl* var_decl_new(TypeVar* type_var, Expr* var_init)
{
	VarDecl* vd = new_s(VarDecl, vd);
	vd->type_var = type_var;
	vd->var_init = var_init;
	return vd;
}

FuncDecl* func_decl_new(const char* func_name, TypeVar** func_params, Type* func_type, Block* func_body)
{
	FuncDecl* fd = new_s(FuncDecl, fd);
	fd->func_name = func_name;
	fd->func_type = func_type;
	fd->func_body = func_body;
	fd->func_params = func_params;
	return fd;
}

LoopStmt* loop_stmt_new(LoopStmtType type, void* loop_stmt_value_ptr)
{
	#define loop_stmt_set_value(type, field) lp->field = (type*)loop_stmt_value_ptr; break
	LoopStmt* lp = new_s(LoopStmt, lp);
	switch (lp->type = type)
	{
	case LOOP_DO:
		loop_stmt_set_value(DoLoop, do_loop);
	case LOOP_FOR:
		loop_stmt_set_value(ForLoop, for_loop);
	case LOOP_WHILE:
		loop_stmt_set_value(WhileLoop, while_loop);
	default:
		assert(0);
	}
	return lp;
}

DoLoop* do_loop_new(Expr* do_cond, Block* do_body)
{
	DoLoop* dl = new_s(DoLoop, dl);
	dl->do_cond = do_cond;
	dl->do_body = do_body;
	return dl;
}

ForLoop* for_loop_new(VarDecl* for_init, Expr* for_cond, Expr* for_step, Block* for_body)
{
	ForLoop* fl = new_s(ForLoop, fl);
	fl->for_init = for_init;
	fl->for_cond = for_cond;
	fl->for_step = for_step;
	fl->for_body = for_body;
	return fl;
}

WhileLoop* while_loop_new(Expr* while_cond, Block* while_body)
{
	WhileLoop* wl = new_s(WhileLoop, wl);
	wl->while_cond = while_cond;
	wl->while_body = while_body;
	return wl;
}

ElseIf* elif_stmt_new(Expr* elif_cond, Block* elif_body)
{
	ElseIf* ei = new_s(ElseIf, ei);
	ei->elif_cond = elif_cond;
	ei->elif_body = elif_body;
	return ei;
}

IfStmt* if_stmt_new(Expr* if_cond, Block* if_body, ElseIf** elifs, Block* else_body)
{
	IfStmt* i = new_s(IfStmt, i);
	i->if_cond = if_cond;
	i->if_body = if_body;
	i->elifs = elifs;
	i->else_body = else_body;
	return i;
}