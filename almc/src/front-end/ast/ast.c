#include "ast.h"

Expr* expr_new(ExprKind type, void* expr_value_ptr)
{
	#define EXPR_SET_VALUE(type, field) \
		expr->field = (type*)expr_value_ptr; break

	Expr* expr = new(Expr);
	switch (expr->kind = type)
	{
	case EXPR_IDNT:
		EXPR_SET_VALUE(Idnt, idnt);
	case EXPR_CONST:
		EXPR_SET_VALUE(Const, cnst);
	case EXPR_STRING:
		EXPR_SET_VALUE(Str, str);
	case EXPR_FUNC_CALL:
		EXPR_SET_VALUE(FuncCall, func_call);
	case EXPR_UNARY_EXPR:
		EXPR_SET_VALUE(UnaryExpr, unary_expr);
	case EXPR_BINARY_EXPR:
		EXPR_SET_VALUE(BinaryExpr, binary_expr);
	case EXPR_TERNARY_EXPR:
		EXPR_SET_VALUE(TernaryExpr, ternary_expr);
	case EXPR_INITIALIZER:
		EXPR_SET_VALUE(Initializer, initializer);
	default:
		report_error("Unexpected expr type in expr_new().", NULL);
	}
	return expr;

	#undef EXPR_SET_VALUE
}

Str* str_new(const char* string, SrcContext* context)
{
	Str* str = new(Str);
	str->svalue = string;
	str->context = context;
	str->type = &unknown_type;
	return str;
}

Idnt* idnt_new(const char* idnt, SrcContext* context)
{
	Idnt* identifier = new(Idnt);
	identifier->svalue = idnt;
	identifier->context = context;
	identifier->type = &unknown_type;
	identifier->is_enum_member = 0;
	identifier->enum_member_value = NULL;
	return identifier;
}

Const* const_new(ConstKind type, const char* svalue, SrcContext* context)
{
#define CONST_CONVERSION(to, type, func)	   \
	if (strlen(svalue) <= 1)				   \
		cnst->to = (type)atof(svalue);		   \
	else									   \
	{								           \
		switch (tolower(svalue[1]))		       \
		{									   \
		case 'x':							   \
			cnst->to = (type)func(svalue + 2, NULL, 16);\
			break;							   \
		case 'o':							   \
			cnst->to = (type)func(svalue + 2, NULL, 8); \
			break;							   \
		case 'b':							   \
			cnst->to = (type)func(svalue + 2, NULL, 2); \
			break;							   \
		default:						       \
			cnst->to = (type)atof(svalue);	   \
		}									   \
	}

	Const* cnst = new(Const);
	cnst->type = &unknown_type;
	cnst->context = context;

	switch (cnst->kind = type)
	{
	case CONST_INT:
		CONST_CONVERSION(ivalue, int64_t, strtoll);
		break;
	case CONST_UINT:
	case CONST_CHAR:
		CONST_CONVERSION(uvalue, uint64_t, strtoull);
		break;
	case CONST_FLOAT:
		cnst->fvalue = strtod(svalue, NULL);
		break;
	}
	return cnst;
}

FuncCall* func_call_new(const char* func_name, Expr** func_args)
{
	FuncCall* func_call = cnew(FuncCall, 1);
	func_call->args = func_args;
	func_call->name = func_name;
	func_call->type = &unknown_type;
	func_call->area = NULL;
	return func_call;
}

UnaryExpr* unary_expr_new(UnaryExprKind type, Expr* expr)
{
	UnaryExpr* unary_expr = new(UnaryExpr);
	unary_expr->kind = type;
	unary_expr->expr = expr;
	unary_expr->type = &unknown_type;
	unary_expr->area = NULL;
	unary_expr->cast_type = NULL;
	return unary_expr;
}

BinaryExpr* binary_expr_new(BinaryExprKind type, Expr* lexpr, Expr* rexpr)
{
	BinaryExpr* binary_expr = new(BinaryExpr);
	binary_expr->kind = type;
	binary_expr->lexpr = lexpr;
	binary_expr->rexpr = rexpr;
	binary_expr->type = &unknown_type;
	binary_expr->area = NULL;
	return binary_expr;
}

TernaryExpr* ternary_expr_new(Expr* cond, Expr* lexpr, Expr* rexpr)
{
	TernaryExpr* ternary_expr = new(TernaryExpr);
	ternary_expr->cond = cond;
	ternary_expr->lexpr = lexpr;
	ternary_expr->rexpr = rexpr;
	ternary_expr->type = &unknown_type;
	ternary_expr->area = NULL;
	return ternary_expr;
}

Initializer* initializer_new(Expr** values)
{
	Initializer* initializer = new(Initializer);
	initializer->area = NULL;
	initializer->type = &unknown_type;
	initializer->values = values;
	return initializer;
}

Stmt* stmt_new(StmtType type, void* stmt_value_ptr)
{
	#define SET_STMT_VALUE(type, field) \
		stmt->field = (type*)stmt_value_ptr; break

	Stmt* stmt = new(Stmt);
	switch (stmt->kind = type)
	{
	case STMT_IF:
		SET_STMT_VALUE(IfStmt, if_stmt);
	case STMT_EXPR:
		SET_STMT_VALUE(ExprStmt, expr_stmt);
	case STMT_BLOCK:
		SET_STMT_VALUE(Block, block);
	case STMT_LOOP:
		SET_STMT_VALUE(LoopStmt, loop_stmt);
	case STMT_JUMP:
		SET_STMT_VALUE(JumpStmt, jump_stmt);
	case STMT_EMPTY:
		SET_STMT_VALUE(EmptyStmt, empty_stmt);
	case STMT_SWITCH:
		SET_STMT_VALUE(SwitchStmt, switch_stmt);
	case STMT_IMPORT:
		SET_STMT_VALUE(ImportStmt, import_stmt);
	case STMT_VAR_DECL:
		SET_STMT_VALUE(VarDecl, var_decl);
	case STMT_TYPE_DECL:
		SET_STMT_VALUE(TypeDecl, type_decl);
	case STMT_FUNC_DECL:
		SET_STMT_VALUE(FuncDecl, func_decl);
	case STMT_LABEL_DECL:
		SET_STMT_VALUE(LabelDecl, label_decl);
	default:
		report_error("Unexpected stmt type.", NULL);
	}
	return stmt;

	#undef SET_STMT_VALUE
}

TypeDecl* type_decl_new(TypeDeclKind type, void* type_decl_value_ptr)
{
	#define TYPE_DECL_SET_VALUE(type, field) \
		type_decl->field = (type*)type_decl_value_ptr; break

	TypeDecl* type_decl = new(TypeDecl);
	switch (type_decl->kind = type)
	{
	case TYPE_DECL_ENUM:
		TYPE_DECL_SET_VALUE(EnumDecl, enum_decl);
	case TYPE_DECL_UNION:
		TYPE_DECL_SET_VALUE(UnionDecl, union_decl);
	case TYPE_DECL_STRUCT:
		TYPE_DECL_SET_VALUE(StructDecl, struct_decl);
	default:
		report_error("Unexpected type-decl type.", NULL);
	}
	return type_decl;

	#undef TYPE_DECL_SET_VALUE
}

EnumDecl* enum_decl_new(EnumMember** members, const char* name)
{
	EnumDecl* enum_decl = new(EnumDecl);
	enum_decl->name = name;
	enum_decl->members = members;
	return enum_decl;
}

UnionDecl* union_decl_new(Member** members, const char* name)
{
	UnionDecl* union_decl = new(UnionDecl);
	union_decl->name = name;
	union_decl->members = members;
	return union_decl;
}

StructDecl* struct_decl_new(Member** members, const char* name)
{
	StructDecl* struct_decl = new(StructDecl);
	struct_decl->name = name;
	struct_decl->members = members;
	struct_decl->alignment = STRUCT_DEFAULT_ALIGNMENT;
	return struct_decl;
}

Member* member_new(char* name, Type* type, SrcArea* area)
{
	Member* member = new(Member);
	member->name = name;
	member->type = type;
	member->area = area;
	member->padding = member->offset = 0;
	return member;
}

EnumMember* enum_member_new(char* name, Expr* value, SrcContext* context)
{
	EnumMember* member = new(EnumMember);
	member->name = name;
	member->value = value;
	member->context = context;
	return member;
}

EmptyStmt* empty_stmt_new()
{
	EmptyStmt* empty_stmt = new(EmptyStmt);
	return empty_stmt;
}

ExprStmt* expr_stmt_new(Expr* expr)
{
	ExprStmt* expr_stmt = new(ExprStmt);
	expr_stmt->expr = expr;
	return expr_stmt;
}

Block* block_new(Stmt** stmts)
{
	Block* block = new(Block);
	block->stmts = stmts;
	return block;
}

TypeVar* type_var_new(Type* type, const char* var)
{
	TypeVar* type_var = new(TypeVar);
	type_var->var = var;
	type_var->type = type;
	type_var->area = NULL;
	return type_var;
}

VarDecl* var_decl_new(bool is_auto, TypeVar* type_var, Expr* init)
{
	VarDecl* var_decl = new(VarDecl);
	var_decl->type_var = type_var;
	var_decl->var_init = init;
	var_decl->is_auto = is_auto;
	return var_decl;
}

FuncDecl* func_decl_new(Idnt* name, TypeVar** params, 
	Type* type, Block* body, FuncSpec* spec, CallConv* conv)
{
	FuncDecl* func_decl = new(FuncDecl);
	func_decl->name = name;
	func_decl->type = type;
	func_decl->body = body;
	func_decl->params = params;
	func_decl->spec = spec;
	func_decl->conv = conv;
	return func_decl;
}

LabelDecl* label_decl_new(Idnt* label)
{
	LabelDecl* loop_decl = new(LabelDecl);
	loop_decl->label = label;
	return loop_decl;
}

LoopStmt* loop_stmt_new(LoopStmtKind type, void* loop_stmt_value_ptr)
{
	#define LOOP_STMT_SET_VALUE(type, field) \
		loop_stmt->field = (type*)loop_stmt_value_ptr; break

	LoopStmt* loop_stmt = new(LoopStmt);
	switch (loop_stmt->kind = type)
	{
	case LOOP_DO:
		LOOP_STMT_SET_VALUE(DoLoop, do_loop);
	case LOOP_FOR:
		LOOP_STMT_SET_VALUE(ForLoop, for_loop);
	case LOOP_WHILE:
		LOOP_STMT_SET_VALUE(WhileLoop, while_loop);
	default:
		report_error("Unexpected loop-stmt type.", NULL);
	}
	return loop_stmt;

	#undef LOOP_STMT_SET_VALUE
}

DoLoop* do_loop_new(Expr* cond, Block* body)
{
	DoLoop* do_loop = new(DoLoop);
	do_loop->cond = cond;
	do_loop->body = body;
	return do_loop;
}

ForLoop* for_loop_new(VarDecl* init, Expr* cond, Expr* step, Block* body)
{
	ForLoop* for_loop = new(ForLoop);
	for_loop->init = init;
	for_loop->cond = cond;
	for_loop->step = step;
	for_loop->body = body;
	return for_loop;
}

WhileLoop* while_loop_new(Expr* cond, Block* body)
{
	WhileLoop* while_loop = new(WhileLoop);
	while_loop->cond = cond;
	while_loop->body = body;
	return while_loop;
}

ElseIf* elif_stmt_new(Expr* cond, Block* body)
{
	ElseIf* elseif = new(ElseIf);
	elseif->cond = cond;
	elseif->body = body;
	return elseif;
}

IfStmt* if_stmt_new(Expr* cond, Block* body, ElseIf** elifs, Block* else_body)
{
	IfStmt* if_stmt = new(IfStmt);
	if_stmt->cond = cond;
	if_stmt->body = body;
	if_stmt->elifs = elifs;
	if_stmt->else_body = else_body;
	return if_stmt;
}

Case* case_stmt_new(Expr* value, Block* body, bool is_conjucted)
{
	Case* case_expr = new(Case);
	case_expr->value = value;
	case_expr->body = body;
	case_expr->is_conjucted = is_conjucted;
	return case_expr;
}

SwitchStmt* switch_stmt_new(Expr* cond, Case** cases, Block* default_case)
{
	SwitchStmt* switch_stmt = new(SwitchStmt);
	switch_stmt->cond = cond;
	switch_stmt->cases = cases;
	switch_stmt->default_case = default_case;
	return switch_stmt;
}

ImportStmt* import_stmt_new(AstRoot* ast)
{
	ImportStmt* import_stmt = new(ImportStmt);
	import_stmt->ast = ast;
	return import_stmt;
}

JumpStmt* jump_stmt_new(JumpStmtKind type, Expr* additional_expr)
{
	JumpStmt* jump_stmt = new(JumpStmt);
	jump_stmt->area = NULL;
	switch (jump_stmt->kind = type)
	{
	case JUMP_BREAK:
	case JUMP_CONTINUE:
		jump_stmt->additional_expr = NULL;
		break;
	case JUMP_GOTO:
	case JUMP_RETURN:
		jump_stmt->additional_expr 
			= additional_expr;
		break;
	default:
		report_error("Unexpected jump-stmt type.", NULL);
	}
	return jump_stmt;
}

void ast_free(AstRoot* root)
{
	if (root)
	{
		for (uint32_t i = 0; i < sbuffer_len(root->stmts); i++)
			stmt_free(root->stmts[i]);
		sbuffer_free(root->stmts);
		free(root);
	}
}

void expr_free(Expr* expr)
{
	if (expr)
	{
		switch (expr->kind)
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
			report_error("Unexpected expr type in expr_free().", NULL);
		}
		free(expr);
	}
}

void str_free(Str* str)
{
	if (str)
		free(str);
}

void idnt_free(Idnt* idnt)
{
	if (idnt)
		free(idnt);
}

void const_free(Const* cnst)
{
	if (cnst)
		free(cnst);
}

void func_call_free(FuncCall* func_call)
{
	if (func_call)
	{
		for (uint32_t i = 0; i < sbuffer_len(func_call->args); i++)
			expr_free(func_call->args[i]);
		sbuffer_free(func_call->args);
		free(func_call->area);
		free(func_call);
	}
}

void unary_expr_free(UnaryExpr* unary_expr)
{
	if (unary_expr)
	{
		// in cases of theses unary expressions
		// each allocates new type, which is not a part
		// of type-var or func-ret-type
		switch (unary_expr->kind)
		{
		case UNARY_CAST:
		case UNARY_SIZEOF:
			type_free(unary_expr->cast_type);
			break;
		case UNARY_ADDRESS:
			if (unary_expr->type)
				free(unary_expr->type);
			break;
		}
		expr_free(unary_expr->expr);
		free(unary_expr->area);
		free(unary_expr);
	}
}

void binary_expr_free(BinaryExpr* binary_expr)
{
	if (binary_expr)
	{
		expr_free(binary_expr->lexpr);
		expr_free(binary_expr->rexpr);
		free(binary_expr->area);
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
		free(ternary_expr->area);
		free(ternary_expr);
	}
}

void initializer_free(Initializer* initializer)
{
	if (initializer)
	{
		for (uint32_t i = 0; i < sbuffer_len(initializer->values); i++)
			expr_free(initializer->values[i]);
		sbuffer_free(initializer->values);
		free(initializer->area);
		free(initializer);
	}
}

void stmt_free(Stmt* stmt)
{
	if (stmt)
	{
		switch (stmt->kind)
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
		case STMT_IMPORT:
			import_stmt_free(stmt->import_stmt);
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
			report_error(frmt("Unexpected stmt kind met"
				" in function: %s", __FUNCTION__), NULL);
		}
		free(stmt);
	}
}

void type_decl_free(TypeDecl* type_decl)
{
	if (type_decl)
	{
		switch (type_decl->kind)
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
			report_error("Unexpected typedecl type.", NULL);
		}
		free(type_decl);
	}
}

void enum_member_free(EnumMember* enum_member)
{
	if (enum_member)
	{
		switch (enum_member->value->kind)
		{
		case EXPR_CONST:
			const_free(enum_member->value->cnst);
			break;
		case EXPR_BINARY_EXPR:
			if (enum_member->value->binary_expr->rexpr->kind == EXPR_CONST)
				const_free(enum_member->value->binary_expr->rexpr->cnst),
					free(enum_member->value->binary_expr);
			else
				expr_free(enum_member->value);
			break;
		default:
			expr_free(enum_member->value);
			break;
		}
		free(enum_member);
	}
}

void enum_decl_free(EnumDecl* enum_decl)
{
	if (enum_decl)
	{
		for (uint32_t i = 0; i < sbuffer_len(enum_decl->members); i++)
			enum_member_free(enum_decl->members[i]);
		sbuffer_free(enum_decl->members);
		free(enum_decl);
	}
}

void union_decl_free(UnionDecl* union_decl)
{
	if (union_decl)
	{
		for (uint32_t i = 0; i < sbuffer_len(union_decl->members); i++)
			member_free(union_decl->members[i]);
		sbuffer_free(union_decl->members);
		free(union_decl);
	}
}

void struct_decl_free(StructDecl* struct_decl)
{
	if (struct_decl)
	{
		for (uint32_t i = 0; i < sbuffer_len(struct_decl->members); i++)
			member_free(struct_decl->members[i]);
		sbuffer_free(struct_decl->members);
		free(struct_decl);
	}
}

void member_free(Member* member)
{
	if (member)
	{
		type_free(member->type);
		free(member->area);
		free(member);
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
		for (uint32_t i = 0; i < sbuffer_len(block->stmts); i++)
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
		free(type_var->area);
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

void func_spec_free(FuncSpec* func_spec)
{
	if (func_spec)
	{
		if (func_spec->proto)
			free(func_spec->proto);
		free(func_spec);
	}
}

void func_decl_free(FuncDecl* func_decl)
{
	if (func_decl)
	{
		idnt_free(func_decl->name);
		type_free(func_decl->type);
		block_free(func_decl->body);
		func_spec_free(func_decl->spec);
		for (uint32_t i = 0; i < sbuffer_len(func_decl->params); i++)
			type_var_free(func_decl->params[i]);
		sbuffer_free(func_decl->params);
		free(func_decl->conv);
		free(func_decl);
	}
}

void label_decl_free(LabelDecl* label_decl)
{
	if (label_decl)
	{
		idnt_free(label_decl->label);
		free(label_decl);
	}
}

void loop_stmt_free(LoopStmt* loop_stmt)
{
	if (loop_stmt)
	{
		switch (loop_stmt->kind)
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
			report_error("Unexpected loop statement type.", NULL);
		}
		free(loop_stmt);
	}
}

void do_loop_free(DoLoop* do_loop)
{
	if (do_loop)
	{
		expr_free(do_loop->cond);
		block_free(do_loop->body);
		free(do_loop);
	}
}

void for_loop_free(ForLoop* for_loop)
{
	if (for_loop)
	{
		var_decl_free(for_loop->init);
		expr_free(for_loop->cond);
		expr_free(for_loop->step);
		block_free(for_loop->body);
		free(for_loop);
	}
}

void while_loop_free(WhileLoop* while_loop)
{
	if (while_loop)
	{
		expr_free(while_loop->cond);
		block_free(while_loop->body);
		free(while_loop);
	}
}


void elif_stmt_free(ElseIf* elif_stmt)
{
	if (elif_stmt)
	{
		expr_free(elif_stmt->cond);
		block_free(elif_stmt->body);
		free(elif_stmt);
	}
}

void if_stmt_free(IfStmt* if_stmt)
{
	if (if_stmt)
	{
		expr_free(if_stmt->cond);
		block_free(if_stmt->body);
		for (uint32_t i = 0; i < sbuffer_len(if_stmt->elifs); i++)
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
		expr_free(case_stmt->value);
		block_free(case_stmt->body);
		free(case_stmt);
	}
}

void switch_stmt_free(SwitchStmt* switch_stmt)
{
	if (switch_stmt)
	{
		expr_free(switch_stmt->cond);
		for (uint32_t i = 0; i < sbuffer_len(switch_stmt->cases); i++)
			case_stmt_free(switch_stmt->cases[i]);
		sbuffer_free(switch_stmt->cases);
		block_free(switch_stmt->default_case);
		free(switch_stmt);
	}
}

void import_stmt_free(ImportStmt* import_stmt)
{
	if (import_stmt)
	{
		ast_free(import_stmt->ast);
		free(import_stmt);
	}
}

void jump_stmt_free(JumpStmt* jump_stmt)
{
	if (jump_stmt)
	{
		switch (jump_stmt->kind)
		{
		case JUMP_BREAK:
		case JUMP_CONTINUE:
			break;
		case JUMP_GOTO:
		case JUMP_RETURN:
			expr_free(jump_stmt->additional_expr);
			break;
		default:
			report_error("Unexpected jump statement kind in jump_stmt_free().", NULL);
		}
		free(jump_stmt->area);
		free(jump_stmt);
	}
}