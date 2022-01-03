#include "ast-printer.h"

void print_ast(AstRoot* ast)
{
	printf("ast-root:\n");
	for (int i = 0; i < sbuffer_len(ast->stmts); i++)
		print_stmt(ast->stmts[i], "");
}

void print_type_spec(Type* type, const char* indent)
{
	const char* type_kind_str[] = {
		[TYPE_ENUM] = "enum",
		[TYPE_UNION] = "union",
		[TYPE_STRUCT] = "struct",
	};

	switch (type->kind)
	{
	case TYPE_ARRAY:
		printf("%sarray\n", indent);
		print_expr(type->dimension, indent);
		print_type_spec(type->base, frmt("%s   ", indent));
		break;
	case TYPE_POINTER:
		printf("%spointer\n", indent);
		print_type_spec(type->base, frmt("%s   ", indent));
		break;
	case TYPE_ENUM:
	case TYPE_UNION:
	case TYPE_STRUCT:
		printf("%s%s: %s\n", indent, type_kind_str[type->kind],
			type->repr);
		break;
	case TYPE_VOID:
		printf("%svoid\n", indent);
		break;
	case TYPE_PRIMITIVE:
		printf("%sprimitive: %s\n", indent, type->repr);
		break;
	case TYPE_INCOMPLETE:
		printf("%sincomplete: %s\n", indent, type->repr);
		break;
	}
	if (type->size)
		printf("%ssize: %d\n", indent, type->size);
	else 
		printf("%ssize: ?\n", indent);
}

void print_type(Type* type, const char* indent)
{
	printfc(ALMC_CONSOLE_DARKRED, 
		"%stype: %s\n", indent, type->repr);
	print_type_spec(type, frmt("%s   ", indent));
}

void print_idnt(Idnt* idnt, const char* indent)
{
	if (idnt->type)
		printfc(ALMC_CONSOLE_GRAY, "%sidnt: %s (%s)\n", indent, 
			idnt->svalue, type_tostr_plain(idnt->type));
	else
		printfc(ALMC_CONSOLE_GRAY, "%sidnt: %s\n",
			indent, idnt->svalue);
	if (idnt->is_enum_member)
		printfc(ALMC_CONSOLE_GRAY,
			"%s   is_enum_member: 1\n", indent);
}

void print_str(Str* str, const char* indent)
{
	if (str->type)
		printfc(ALMC_CONSOLE_WHITE, "%sstr: \"%s\" (%s)\n", 
			indent, str->svalue, type_tostr_plain(str->type));
	else
		printfc(ALMC_CONSOLE_WHITE, 
			"%sstr: \"%s\"\n", indent, str->svalue);
}

void print_const(Const* cnst, const char* indent)
{
	switch (cnst->kind)
	{
	case CONST_INT:
		printfc(ALMC_CONSOLE_GRAY, "%sint-const: %lld",
			indent, cnst->ivalue);
		break;
	case CONST_UINT:
		printfc(ALMC_CONSOLE_GRAY, "%suint-const: %I64u",
			indent, cnst->uvalue);
		break;
	case CONST_FLOAT:
		printfc(ALMC_CONSOLE_GRAY, "%sfloat-const: %lf",
			indent, cnst->fvalue);
		break;
	case CONST_CHAR:
		printfc(ALMC_CONSOLE_GRAY, "%schar-const: %c (%d)", indent,
			(char)cnst->uvalue, (char)cnst->uvalue);
	}
	if (cnst->type)
		printfc(ALMC_CONSOLE_GRAY, " (%s)\n",
			type_tostr_plain(cnst->type));
	else
		printf("\n");
}

void print_func_call(FuncCall* func_call, const char* indent)
{
	size_t len = sbuffer_len(func_call->args);
	if (func_call->type)
		printfc(ALMC_CONSOLE_GREEN, "%sfunc-call: %s(args: %d) %s\n", indent,
			func_call->name, len, type_tostr_plain(func_call->type));
	else
		printfc(ALMC_CONSOLE_GREEN, "%sfunc-call: %s(args: %d)\n",
			indent, func_call->name, len);
	if (len)
		for (int i = 0; i < len; i++)
			print_expr(func_call->args[i], indent);
	else
		printf("%s   no-args\n", indent);
}

void print_unary_expr(UnaryExpr* expr, const char* indent)
{
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
		"unary-lengthof:",

		"unary-postfix-inc: ++",
		"unary-postfix-dec: --",
	};

	switch (expr->kind)
	{
	case UNARY_CAST:
	case UNARY_SIZEOF:
		if (expr->type)
			printfc(ALMC_CONSOLE_DARKYELLOW, "%s%s (%s)\n", indent,
				unary_expr_type_str[expr->kind], type_tostr_plain(expr->type));
		else
			printfc(ALMC_CONSOLE_DARKYELLOW, "%s%s\n", indent, 
				unary_expr_type_str[expr->kind]);
		if (expr->cast_type)
			print_type(expr->cast_type, frmt("%s   ", indent));
		break;
	default:
		if (expr->type)
			printfc(ALMC_CONSOLE_DARKYELLOW, "%s%s (%s)\n", indent,
				unary_expr_type_str[expr->kind], type_tostr_plain(expr->type));
		else
			printfc(ALMC_CONSOLE_DARKYELLOW, "%s%s\n", indent,
				unary_expr_type_str[expr->kind]);
	}
	print_expr(expr->expr, indent);
}

void print_binary_expr(BinaryExpr* expr, const char* indent)
{
	const char* binary_expr_type_str[] = {
		"binary-add: +",
		"binary-sub: -",
		"binary-div: /",
		"binary-mod: %",
		"binary-mult: *",

		"binary-comma: ,",
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
	if (expr->type)
		printfc(ALMC_CONSOLE_YELLOW, "%s%s (%s)\n", 
			indent, binary_expr_type_str[expr->kind], type_tostr_plain(expr->type));
	else
		printfc(ALMC_CONSOLE_YELLOW, "%s%s\n", 
			indent, binary_expr_type_str[expr->kind]);
	print_expr(expr->lexpr, indent);
	print_expr(expr->rexpr, indent);
}

void print_ternary_expr(TernaryExpr* expr, const char* indent)
{
	if (expr->type)
		printfc(ALMC_CONSOLE_GREEN, "%sternary: (%s)\n", indent,
			type_tostr_plain(expr->type));
	else
		printfc(ALMC_CONSOLE_GREEN, "%sternary: \n", indent);
	print_expr(expr->cond, indent);
	print_expr(expr->lexpr, indent);
	print_expr(expr->rexpr, indent);
}

void print_initializer(Initializer* initializer, const char* indent)
{
	printfc(ALMC_CONSOLE_GREEN, "%sinitializer:\n", indent);
	indent = frmt("%s   ", indent);

	for (int i = 0; i < sbuffer_len(initializer->values); i++)
		printf("%sitem %d:\n", indent, i + 1), 
			print_expr(initializer->values[i], indent);
}

void print_expr(Expr* expr, const char* indent)
{
	// |——
	char* new_indent = frmt("%s   ", indent);
	if (!expr)
		printf("%s   no-body\n", indent);
	else
	{
		switch (expr->kind)
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
		case EXPR_INITIALIZER:
			print_initializer(expr->initializer, new_indent);
			break;
		}
	}
}

void print_type_var(TypeVar* type_var, const char* indent)
{
	if (type_var->type)
		printfc(ALMC_CONSOLE_DARKCYAN, "%stype-var: %s (%s)\n", 
			indent, type_var->var, type_tostr_plain(type_var->type));
	else
		printfc(ALMC_CONSOLE_DARKCYAN, "%stype-var: %s\n", 
			indent, type_var->var);
	print_type(type_var->type, frmt("   %s", indent));
}

void print_block(Block* block, const char* indent)
{
	printfc(ALMC_CONSOLE_CYAN, "%sblock:\n", indent);
	if (!sbuffer_len(block->stmts))
		printf("%s   empty-block\n", indent);
	for (int i = 0; i < sbuffer_len(block->stmts); i++)
		print_stmt(block->stmts[i], indent);
}

void print_var_decl(VarDecl* var_decl, const char* indent)
{
	printfc(ALMC_CONSOLE_DARKCYAN, "%svar-decl:\n", indent);
	indent = frmt("   %s", indent);
	if (!var_decl)
		printf("%sno-decl\n", indent);
	else
	{
		print_type_var(var_decl->type_var, indent);
		printfc(ALMC_CONSOLE_DARKCYAN, "%svar-decl-init:\n", indent);
		print_expr(var_decl->var_init, indent);
	}
}

void print_func_decl(FuncDecl* func_decl, const char* indent)
{
#define print_func_spec(x) func_decl->spec->x ? \
	printf("%s%s: %d\n", indent, #x, func_decl->spec->x) : 0

	printfc(ALMC_CONSOLE_CYAN, "%sfunc-decl: %s\n", indent,
		func_decl->name->svalue);
	indent = frmt("   %s", indent);
	print_func_spec(is_entry);
	print_func_spec(is_vararg);
	print_func_spec(is_external);

	printfc(ALMC_CONSOLE_CYAN, "%sfunc-ret-type:\n", indent);
	print_type(func_decl->type, frmt("   %s", indent));

	printfc(ALMC_CONSOLE_CYAN, "%sfunc-params:\n", indent);
	if (!sbuffer_len(func_decl->params))
		printf("%s   no-params\n", indent);
	for (int i = 0; i < sbuffer_len(func_decl->params); i++)
		print_type_var(func_decl->params[i], frmt("   %s", indent));

	if (func_decl->body)
	{
		printfc(ALMC_CONSOLE_CYAN, "%sfunc-body:\n", indent);
		print_block(func_decl->body, frmt("   %s", indent));
	}
}

void print_label_decl(LabelDecl* label_decl, const char* indent)
{
	printfc(ALMC_CONSOLE_CYAN, "%slabel-decl: %s\n", 
		indent, label_decl->label->svalue);
}

void print_member(Member* member, const char* indent)
{
	printfc(ALMC_CONSOLE_DARKCYAN, "%smember: %s (%d:%d)\n", 
		indent, member->name, member->padding, member->offset);
	print_type(member->type, frmt("   %s", indent));
}

void print_enum_member(EnumMember* member, const char* indent)
{
	printf("%s   %s:\n", indent, member->name);
	print_expr(member->value, frmt("   %s", indent));
}

void print_enum_decl(EnumDecl* enum_decl, const char* indent)
{
	printfc(ALMC_CONSOLE_MAGENTA, "%senum-decl: %s{idnts: %d}\n",
		indent, enum_decl->name, sbuffer_len(enum_decl->members));
	for (int i = 0; i < sbuffer_len(enum_decl->members); i++)
		print_enum_member(enum_decl->members[i], indent);
}

void print_union_decl(UnionDecl* union_decl, const char* indent)
{
	printfc(ALMC_CONSOLE_MAGENTA, "%sunion-decl: %s{mmbrs: %d}\n", 
		indent, union_decl->name, sbuffer_len(union_decl->members));
	for (int i = 0; i < sbuffer_len(union_decl->members); i++)
		print_member(union_decl->members[i], frmt("   %s", indent));
}

void print_struct_decl(StructDecl* struct_decl, const char* indent)
{
	printfc(ALMC_CONSOLE_MAGENTA, "%sstruct-decl: %s{mmbrs: %d}\n", 
		indent, struct_decl->name, sbuffer_len(struct_decl->members));
	for (int i = 0; i < sbuffer_len(struct_decl->members); i++)
		print_member(struct_decl->members[i], frmt("   %s", indent));
}

void print_type_decl(TypeDecl* type_decl, const char* indent)
{
	switch (type_decl->kind)
	{
	case TYPE_DECL_ENUM:
		print_enum_decl(type_decl->enum_decl, indent);
		break;
	case TYPE_DECL_UNION:
		print_union_decl(type_decl->union_decl, indent);
		break;
	case TYPE_DECL_STRUCT:
		print_struct_decl(type_decl->struct_decl, indent);
		break;
	default:
		report_error("Unexpcted type declaration type.", NULL);
	}
}

void print_do_loop(DoLoop* do_loop, const char* indent)
{
	printfc(ALMC_CONSOLE_MAGENTA, "%sdo-loop:\n", indent);
	indent = frmt("   %s", indent);

	printfc(ALMC_CONSOLE_CYAN, "%sdo-body:\n", indent);
	print_block(do_loop->body, frmt("   %s", indent));

	printfc(ALMC_CONSOLE_CYAN, "%sdo-cond:\n", indent);
	print_expr(do_loop->cond, indent);
}

void print_for_loop(ForLoop* for_loop, const char* indent)
{
	printfc(ALMC_CONSOLE_MAGENTA, "%sfor-loop:\n", indent);
	indent = frmt("   %s", indent);
	printfc(ALMC_CONSOLE_CYAN, "%sfor-init:\n", indent);
	print_var_decl(for_loop->init, frmt("   %s", indent));

	printfc(ALMC_CONSOLE_CYAN, "%sfor-cond:\n", indent);
	print_expr(for_loop->cond, indent);

	printfc(ALMC_CONSOLE_CYAN, "%sfor-step:\n", indent);
	print_expr(for_loop->step, indent);

	printfc(ALMC_CONSOLE_CYAN, "%sfor-body:\n", indent);
	print_block(for_loop->body, frmt("   %s", indent));
}

void print_while_loop(WhileLoop* while_loop, const char* indent)
{
	printfc(ALMC_CONSOLE_MAGENTA, "%swhile-loop:\n", indent);
	indent = frmt("   %s", indent);

	printfc(ALMC_CONSOLE_CYAN, "%swhile-cond:\n", indent);
	print_expr(while_loop->cond, indent);

	printfc(ALMC_CONSOLE_CYAN, "%swhile-body:\n", indent);
	print_block(while_loop->body, frmt("   %s", indent));
}

void print_loop_stmt(LoopStmt* loop_stmt, const char* indent)
{
	switch (loop_stmt->kind)
	{
	case LOOP_DO:
		print_do_loop(loop_stmt->do_loop, indent);
		break;
	case LOOP_FOR:
		print_for_loop(loop_stmt->for_loop, indent);
		break;
	case LOOP_WHILE:
		print_while_loop(loop_stmt->while_loop, indent);
		break;
	default:
		report_error("Unknown loop kind met in print_loop_stmt()", NULL);
	}
}

void print_expr_stmt(ExprStmt* expr_stmt, const char* indent)
{
	printfc(ALMC_CONSOLE_DARKCYAN, "%sexpr-stmt:\n", indent);
	print_expr(expr_stmt->expr, indent);
}

void print_empty_stmt(EmptyStmt* empty_stmt, const char* indent)
{
	printfc(ALMC_CONSOLE_BLUE, "%sempty-stmt\n", indent);
}

void print_if_stmt(IfStmt* if_stmt, const char* indent)
{
	printfc(ALMC_CONSOLE_MAGENTA, "%sif-stmt:\n", indent);
	char* indent2 = frmt("   %s", indent);

	printfc(ALMC_CONSOLE_CYAN, "%sif-cond:\n", indent2);
	print_expr(if_stmt->cond, indent2);

	printfc(ALMC_CONSOLE_CYAN, "%sif-body:\n", indent2);
	print_block(if_stmt->body, frmt("   %s", indent2));

	for (int i = 0; i < sbuffer_len(if_stmt->elifs); i++)
	{
		printfc(ALMC_CONSOLE_MAGENTA, "%selif-stmt:\n", indent);

		printfc(ALMC_CONSOLE_CYAN, "%selif-cond:\n", indent2);
		print_expr(if_stmt->elifs[i]->cond, indent2);

		printfc(ALMC_CONSOLE_CYAN, "%selif-body:\n", indent2);
		print_block(if_stmt->elifs[i]->body, frmt("   %s", indent2));
	}

	if (if_stmt->else_body)
	{
		printfc(ALMC_CONSOLE_MAGENTA, "%selse-stmt:\n", indent);
		print_block(if_stmt->else_body, frmt("   %s", indent));
	}
}

void print_jump_stmt(JumpStmt* jump_stmt, const char* indent)
{
	printfc(ALMC_CONSOLE_DARKMAGENTA, indent);
	switch (jump_stmt->kind)
	{
	case JUMP_GOTO:
		printfc(ALMC_CONSOLE_DARKMAGENTA, "goto-stmt:\n");
		print_expr(jump_stmt->additional_expr, indent);
		break;
	case JUMP_BREAK:
		printfc(ALMC_CONSOLE_DARKMAGENTA, "break-stmt\n");
		break;
	case JUMP_RETURN:
		printfc(ALMC_CONSOLE_DARKMAGENTA, "return-stmt\n");
		if (jump_stmt->additional_expr)
			print_expr(jump_stmt->additional_expr, indent);
		break;
	case JUMP_CONTINUE:
		printfc(ALMC_CONSOLE_DARKMAGENTA, "continue-stmt\n");
		break;
	default:
		report_error("Unknown jump statemnt kind met in print_jump_stmt()", NULL);
	}
}

void print_case_stmt(Case* case_stmt, const char* indent)
{
	printfc(ALMC_CONSOLE_MAGENTA, "%scase-stmt:\n", indent);
	indent = frmt("%s   ", indent);
	printfc(ALMC_CONSOLE_MAGENTA, "%scase-value:\n", indent);
	print_expr(case_stmt->value, indent);

	if (case_stmt->body)
	{
		printfc(ALMC_CONSOLE_MAGENTA, "%scase-body:\n", indent);
		print_block(case_stmt->body, frmt("%s   ", indent));
	}
}

void print_switch_stmt(SwitchStmt* switch_stmt, const char* indent)
{
	printfc(ALMC_CONSOLE_MAGENTA, "%sswitch-stmt:\n", indent);
	indent = frmt("%s   ", indent);

	printfc(ALMC_CONSOLE_MAGENTA, "%sswitch-cond:\n", indent);
	print_expr(switch_stmt->cond, indent);

	for (int i = 0; i < sbuffer_len(switch_stmt->cases); i++)
		print_case_stmt(switch_stmt->cases[i], frmt("%s   ", indent));

	if (switch_stmt->default_case)
	{
		printfc(ALMC_CONSOLE_MAGENTA, "%sdefault-stmt:\n", indent);
		print_block(switch_stmt->default_case, frmt("%s   ", indent));
	}
}

void print_import_stmt(ImportStmt* import_stmt, const char* indent)
{
	printfc(ALMC_CONSOLE_RED, "%simport-stmt:\n", indent);
	for (int i = 0; i < sbuffer_len(import_stmt->ast->stmts); i++)
		print_stmt(import_stmt->ast->stmts[i], indent);
}

void print_stmt(Stmt* stmt, const char* indent)
{
	char* new_indent = frmt("%s   ", indent);
	if (!stmt)
		printf("%s   no-body\n", indent);
	else
	{
		switch (stmt->kind)
		{
		case STMT_IF:
			print_if_stmt(stmt->if_stmt, new_indent);
			break;
		case STMT_EXPR:
			print_expr_stmt(stmt->expr_stmt, new_indent);
			break;
		case STMT_BLOCK:
			print_block(stmt->block, new_indent);
			break;
		case STMT_LOOP:
			print_loop_stmt(stmt->loop_stmt, new_indent);
			break;
		case STMT_JUMP:
			print_jump_stmt(stmt->jump_stmt, new_indent);
			break;
		case STMT_EMPTY:
			print_empty_stmt(stmt->empty_stmt, new_indent);
			break;
		case STMT_SWITCH:
			print_switch_stmt(stmt->switch_stmt, new_indent);
			break;
		case STMT_IMPORT:
			print_import_stmt(stmt->import_stmt, new_indent);
			break;
		case STMT_VAR_DECL:
			print_var_decl(stmt->var_decl, new_indent);
			break;
		case STMT_TYPE_DECL:
			print_type_decl(stmt->type_decl, new_indent);
			break;
		case STMT_FUNC_DECL:
			print_func_decl(stmt->func_decl, new_indent);
			break;
		case STMT_LABEL_DECL:
			print_label_decl(stmt->label_decl, new_indent);
			break;
		default:
			report_error("Unknown statemnt kind met in print_stmt()", NULL);
		}
	}
}