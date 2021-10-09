#include "ast.h"

//todo: replace assert(0) by report_error

Expr* expr_new(ExprType type, void* expr_value_ptr)
{
	#define expr_set_value(type, field) e->field = (type*)expr_value_ptr; break
	Expr* e = new_s(Expr, e);
	switch (e->type = type)
	{
	case EXPR_IDNT:
		expr_set_value(Idnt, idnt);
	case EXPR_CONST:
		expr_set_value(Const, cnst);
	case EXPR_STRING:
		expr_set_value(Str, str);
	case EXPR_FUNC_CALL:
		expr_set_value(FuncCall, func_call);
	case EXPR_UNARY_EXPR:
		expr_set_value(UnaryExpr, unary_expr);
	case EXPR_BINARY_EXPR:
		expr_set_value(BinaryExpr, binary_expr);
	case EXPR_TERNARY_EXPR:
		expr_set_value(TernaryExpr, ternary_expr);
	case EXPR_INITIALIZER:
		expr_set_value(Initializer, initializer);
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

Initializer* initializer_new(Expr** values)
{
	Initializer* si = new_s(Initializer, si);
	si->values = values;
	return si;
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
	case STMT_JUMP:
		stmt_set_value(JumpStmt, jump_stmt);
	case STMT_EMPTY:
		stmt_set_value(EmptyStmt, empty_stmt);
	case STMT_SWITCH:
		stmt_set_value(SwitchStmt, switch_stmt);
	case STMT_VAR_DECL:
		stmt_set_value(VarDecl, var_decl);
	case STMT_TYPE_DECL:
		stmt_set_value(TypeDecl, type_decl);
	case STMT_FUNC_DECL:
		stmt_set_value(FuncDecl, func_decl);
	case STMT_LABEL_DECL:
		stmt_set_value(LabelDecl, label_decl);
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

LabelDecl* label_decl_new(Idnt* label_idnt)
{
	LabelDecl* ld = new_s(LabelDecl, ld);
	ld->label_idnt = label_idnt;
	return ld;
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

Case* case_stmt_new(Expr* case_value, Block* case_body)
{
	Case* c = new_s(Case, c);
	c->case_value = case_value;
	c->case_body = case_body;
	return c;
}

SwitchStmt* switch_stmt_new(Expr* switch_cond, Case** switch_cases, Block* switch_default)
{
	SwitchStmt* ss = new_s(SwitchStmt, ss);
	ss->switch_cond = switch_cond;
	ss->switch_cases = switch_cases;
	ss->switch_default = switch_default;
	return ss;
}

JumpStmt* jump_stmt_new(JumpStmtType type, Expr* additional_expr)
{
	JumpStmt* js = new_s(JumpStmt, js);
	switch (js->type = type)
	{
	case JUMP_BREAK:
	case JUMP_CONTINUE:
		js->additional_expr = NULL;
		break;
	case JUMP_GOTO:
	case JUMP_RETURN:
		js->additional_expr 
			= additional_expr;
		break;
	default:
		assert(0);
	}
	return js;
}

//todo: still have mem leak, but not so big as before
void type_free(Type* type)
{
	if (type)
	{ 
		//free(type->repr);
		free(type);
	}
}

void expr_free(Expr* expr)
{
	if (expr)
	{
		switch (expr->type)
		{
		case EXPR_IDNT:
			idnt_free(expr->idnt);
			break;
		case EXPR_CONST:
			const_free(expr->cnst);
			break;
		case EXPR_STRING:
			str_free(expr->str);
			break;
		case EXPR_FUNC_CALL:
			func_call_free(expr->func_call);
			break;
		case EXPR_UNARY_EXPR:
			unary_expr_free(expr->unary_expr);
			break;
		case EXPR_BINARY_EXPR:
			binary_expr_free(expr->binary_expr);
			break;
		case EXPR_TERNARY_EXPR:
			ternary_expr_free(expr->ternary_expr);
			break;
		case EXPR_INITIALIZER:
			initializer_free(expr->initializer);
			break;
		default:
			assert(0);
		}
		free(expr);
	}
}

void str_free(Str* str)
{
	if (str)
	{
		//free(str->svalue);
		src_context_free(str->context);
		free(str);
	}
}

void idnt_free(Idnt* idnt)
{
	if (idnt)
	{
		//free(idnt->svalue);
		src_context_free(idnt->context);
		free(idnt);
	}
}

void const_free(Const* cnst)
{
	if (cnst)
	{
		src_context_free(cnst->context);
		free(cnst);
	}
}

void func_call_free(FuncCall* func_call)
{
	if (func_call)
	{
		for (int i = 0; i < sbuffer_len(func_call->func_args); i++)
			expr_free(func_call->func_args[i]);
		sbuffer_free(func_call->func_args);
		//free(func_call->func_name);
		free(func_call);
	}
}

void unary_expr_free(UnaryExpr* unary_expr)
{
	if (unary_expr)
	{
		expr_free(unary_expr->expr);
		type_free(unary_expr->cast_type);
		free(unary_expr);
	}
}

void binary_expr_free(BinaryExpr* binary_expr)
{
	if (binary_expr)
	{
		expr_free(binary_expr->lexpr);
		expr_free(binary_expr->rexpr);
		free(binary_expr);
	}
}

void ternary_expr_free(TernaryExpr* ternary_expr)
{
	if (ternary_expr)
	{
		expr_free(ternary_expr->cond);
		expr_free(ternary_expr->lexpr);
		expr_free(ternary_expr->rexpr);
		free(ternary_expr);
	}
}

void initializer_free(Initializer* initializer)
{
	if (initializer)
	{
		for (int i = 0; i < sbuffer_len(initializer->values); i++)
			expr_free(initializer->values[i]);
		sbuffer_free(initializer->values);
		free(initializer);
	}
}

void stmt_free(Stmt* stmt)
{
	if (stmt)
	{
		switch (stmt->type)
		{
		case STMT_IF:
			if_stmt_free(stmt->if_stmt);
			break;
		case STMT_EXPR:
			expr_stmt_free(stmt->expr_stmt);
			break;
		case STMT_BLOCK:
			block_free(stmt->block);
			break;
		case STMT_LOOP:
			loop_stmt_free(stmt->loop_stmt);
			break;
		case STMT_JUMP:
			jump_stmt_free(stmt->jump_stmt);
			break;
		case STMT_EMPTY:
			empty_stmt_free(stmt->empty_stmt);
			break;
		case STMT_SWITCH:
			switch_stmt_free(stmt->switch_stmt);
			break;
		case STMT_VAR_DECL:
			var_decl_free(stmt->var_decl);
			break;
		case STMT_TYPE_DECL:
			type_decl_free(stmt->type_decl);
			break;
		case STMT_FUNC_DECL:
			func_decl_free(stmt->func_decl);
			break;
		case STMT_LABEL_DECL:
			label_decl_free(stmt->label_decl);
			break;
		default:
			assert(0);
		}
		free(stmt);
	}
}

void type_decl_free(TypeDecl* type_decl)
{
	if (type_decl)
	{
		switch (type_decl->type)
		{
		case TYPE_DECL_ENUM:
			enum_decl_free(type_decl->enum_decl);
			break;
		case TYPE_DECL_UNION:
			union_decl_free(type_decl->union_decl);
			break;
		case TYPE_DECL_STRUCT:
			struct_decl_free(type_decl->struct_decl);
			break;
		default:
			assert(0);
		}
		free(type_decl);
	}
}

void enum_decl_free(EnumDecl* enum_decl)
{
	if (enum_decl)
	{
		//free(enum_decl->enum_name);
		for (int i = 0; i < sbuffer_len(enum_decl->enum_idnts); i++)
		{
			idnt_free(enum_decl->enum_idnts[i]);
			expr_free(enum_decl->enum_idnt_values[i]);
		}
		sbuffer_free(enum_decl->enum_idnts);
		free(enum_decl);
	}
}

void union_decl_free(UnionDecl* union_decl)
{
	if (union_decl)
	{
		//free(union_decl->union_name);
		for (int i = 0; i < sbuffer_len(union_decl->union_mmbrs); i++)
			type_var_free(union_decl->union_mmbrs[i]);
		sbuffer_free(union_decl->union_mmbrs);
		free(union_decl);
	}
}

void struct_decl_free(StructDecl* struct_decl)
{
	if (struct_decl)
	{
		//free(struct_decl->struct_name);
		for (int i = 0; i < sbuffer_len(struct_decl->struct_mmbrs); i++)
			type_var_free(struct_decl->struct_mmbrs[i]);
		sbuffer_free(struct_decl->struct_mmbrs);
		free(struct_decl);
	}
}

void empty_stmt_free(EmptyStmt* empty_stmt)
{
	if (empty_stmt)
		free(empty_stmt);
}

void expr_stmt_free(ExprStmt* expr_stmt)
{
	if (expr_stmt)
	{
		expr_free(expr_stmt->expr);
		free(expr_stmt);
	}
}

void block_free(Block* block)
{
	if (block)
	{
		for (int i = 0; i < sbuffer_len(block->stmts); i++)
			stmt_free(block->stmts[i]);
		sbuffer_free(block->stmts);
		free(block);
	}
}

void type_var_free(TypeVar* type_var)
{
	if (type_var)
	{
		type_free(type_var->type);
		//free(type_var->var);
		free(type_var);
	}
}

void var_decl_free(VarDecl* var_decl)
{
	if (var_decl)
	{
		expr_free(var_decl->var_init);
		type_var_free(var_decl->type_var);
		free(var_decl);
	}
}

void func_decl_free(FuncDecl* func_decl)
{
	if (func_decl)
	{
		type_free(func_decl->func_type);
		block_free(func_decl->func_body);
		for (int i = 0; i < sbuffer_len(func_decl->func_params); i++)
			type_var_free(func_decl->func_params[i]);
		sbuffer_free(func_decl->func_params);
		//free(func_decl->func_name);
		free(func_decl);
	}
}

void label_decl_free(LabelDecl* label_decl)
{
	if (label_decl)
	{
		idnt_free(label_decl->label_idnt);
		free(label_decl);
	}
}

void loop_stmt_free(LoopStmt* loop_stmt)
{
	if (loop_stmt)
	{
		switch (loop_stmt->type)
		{
		case LOOP_DO:
			do_loop_free(loop_stmt->do_loop);
			break;
		case LOOP_FOR:
			for_loop_free(loop_stmt->for_loop);
			break;
		case LOOP_WHILE:
			while_loop_free(loop_stmt->while_loop);
			break;
		default:
			assert(0);
		}
		free(loop_stmt);
	}
}

void do_loop_free(DoLoop* do_loop)
{
	if (do_loop)
	{
		expr_free(do_loop->do_cond);
		block_free(do_loop->do_body);
		free(do_loop);
	}
}

void for_loop_free(ForLoop* for_loop)
{
	if (for_loop)
	{
		var_decl_free(for_loop->for_init);
		expr_free(for_loop->for_cond);
		expr_free(for_loop->for_step);
		block_free(for_loop->for_body);
		free(for_loop);
	}
}

void while_loop_free(WhileLoop* while_loop)
{
	if (while_loop)
	{
		expr_free(while_loop->while_cond);
		block_free(while_loop->while_body);
		free(while_loop);
	}
}


void elif_stmt_free(ElseIf* elif_stmt)
{
	if (elif_stmt)
	{
		expr_free(elif_stmt->elif_cond);
		block_free(elif_stmt->elif_body);
		free(elif_stmt);
	}
}

void if_stmt_free(IfStmt* if_stmt)
{
	if (if_stmt)
	{
		expr_free(if_stmt->if_cond);
		block_free(if_stmt->if_body);
		for (int i = 0; i < sbuffer_len(if_stmt->elifs); i++)
			elif_stmt_free(if_stmt->elifs[i]);
		sbuffer_free(if_stmt->elifs);
		block_free(if_stmt->else_body);
		free(if_stmt);
	}
}

void case_stmt_free(Case* case_stmt)
{
	if (case_stmt)
	{
		expr_free(case_stmt->case_value);
		block_free(case_stmt->case_body);
		free(case_stmt);
	}
}

void switch_stmt_free(SwitchStmt* switch_stmt)
{
	if (switch_stmt)
	{
		expr_free(switch_stmt->switch_cond);
		for (int i = 0; i < sbuffer_len(switch_stmt->switch_cases); i++)
			case_stmt_free(switch_stmt->switch_cases[i]);
		sbuffer_free(switch_stmt->switch_cases);
		block_free(switch_stmt->switch_default);
		free(switch_stmt);
	}
}

void jump_stmt_free(JumpStmt* jump_stmt)
{
	if (jump_stmt)
	{
		switch (jump_stmt->type)
		{
		case JUMP_BREAK:
		case JUMP_CONTINUE:
			break;
		case JUMP_GOTO:
		case JUMP_RETURN:
			expr_free(jump_stmt->additional_expr);
			break;
		default:
			assert(0);
		}
		free(jump_stmt);
	}
}