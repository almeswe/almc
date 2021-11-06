#include "ast-console-printer.h"

void print_ast(AstRoot* ast)
{
	printf("ast-root:\n");
	for (int i = 0; i < sbuffer_len(ast->stmts); i++)
		print_stmt(ast->stmts[i], "");
}

void print_type(Type* type, const char* indent)
{
#define print_type_mode(mode) type->mods.mode ? \
	printf("%s%s: %d\n", indent, #mode, type->mods.mode) : 0

	printf(BOLDRED);
	printf("%stype: %s\n", indent, type->repr);
	indent = frmt("%s   ", indent);
	printf(RESET);
	print_type_mode(is_ptr);
	print_type_mode(is_void);
	print_type_mode(is_static);
	print_type_mode(is_register);
	print_type_mode(is_volatile);
	print_type_mode(is_const_ptr);
	print_type_mode(is_predefined);
	print_type_mode(is_array);
	indent = frmt("%s   ", indent);
	for (int i = 0; i < sbuffer_len(type->info.arr_dim_sizes); i++)
	{
		printf("%sdim %d:\n", indent, i + 1);
		print_expr(type->info.arr_dim_sizes[i], indent);
	}
}

void print_idnt(Idnt* idnt, const char* indent)
{
	printf(BOLDWHITE);
	printf("%sidnt: %s\n", indent, idnt->svalue);
	printf(RESET);
}

void print_str(Str* str, const char* indent)
{
	printf(WHITE);
	printf("%sstr: \"%s\"\n", indent, str->svalue);
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

void print_initializer(Initializer* initializer, const char* indent)
{
	printf(BOLDGREEN);
	printf("%sinitializer:\n", indent);
	indent = frmt("%s   ", indent);
	printf(RESET);
	for (int i = 0; i < sbuffer_len(initializer->values); i++)
	{
		printf("%sitem %d:\n", indent, i + 1);
		print_expr(initializer->values[i], indent);
	}
}

void print_expr(Expr* expr, const char* indent)
{
	// |��
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
		case EXPR_INITIALIZER:
			print_initializer(expr->initializer, new_indent);
			break;
		}
	else
		printf("%s   no-body\n", indent);
}

void print_type_var(TypeVar* type_var, const char* indent)
{
	printf(CYAN);
	printf("%stype-var: %s\n", indent, type_var->var);
	printf(RESET);
	print_type(type_var->type, frmt("   %s", indent));
}

void print_block(Block* block, const char* indent)
{
	printf(BOLDCYAN);
	printf("%sblock:\n", indent);
	printf(RESET);
	if (!sbuffer_len(block->stmts))
		printf("%s   empty-block\n", indent);
	for (int i = 0; i < sbuffer_len(block->stmts); i++)
		print_stmt(block->stmts[i], indent);
}

void print_var_decl(VarDecl* var_decl, const char* indent)
{
	printf(CYAN);
	printf("%svar-decl:\n", indent);
	printf(RESET);
	indent = frmt("   %s", indent);
	if (!var_decl)
		printf("%sno-decl\n", indent);
	else
	{
		print_type_var(var_decl->type_var, indent);
		printf(CYAN);
		printf("%svar-decl-init:\n", indent);
		printf(RESET);
		print_expr(var_decl->var_init, indent);
	}
}

void print_func_decl(FuncDecl* func_decl, const char* indent)
{
#define print_func_spec(spec) func_decl->func_spec.spec ? \
	printf("%s%s: %d\n", indent, #spec, func_decl->func_spec.spec) : 0

	printf(BOLDCYAN);
	printf("%sfunc-decl: %s\n", indent, func_decl->func_name);
	indent = frmt("   %s", indent);
	printf(RESET);
	print_func_spec(is_forward);
	print_func_spec(is_external);
	print_func_spec(is_intrinsic);

	printf(BOLDCYAN);
	printf("%sfunc-ret-type:\n", indent);
	printf(RESET);
	print_type(func_decl->func_type, frmt("   %s", indent));

	printf(BOLDCYAN);
	printf("%sfunc-params:\n", indent);
	printf(RESET);
	if (!sbuffer_len(func_decl->func_params))
		printf("%s   no-params\n", indent);
	for (int i = 0; i < sbuffer_len(func_decl->func_params); i++)
		print_type_var(func_decl->func_params[i], frmt("   %s", indent));

	if (func_decl->func_body)
	{
		printf(BOLDCYAN);
		printf("%sfunc-body:\n", indent);
		printf(RESET);
		print_block(func_decl->func_body, frmt("   %s", indent));
	}
}

void print_label_decl(LabelDecl* label_decl, const char* indent)
{
	printf(BOLDCYAN);
	printf("%slabel-decl: %s\n", indent, label_decl->label_idnt->svalue);
	printf(RESET);
}

void print_enum_decl(EnumDecl* enum_decl, const char* indent)
{
	printf(BOLDMAGENTA);
	printf("%senum-decl: %s{idnts: %d}\n", indent, enum_decl->enum_name, sbuffer_len(enum_decl->enum_idnts));
	printf(RESET);
	for (int i = 0; i < sbuffer_len(enum_decl->enum_idnts); i++)
	{
		printf("%s   %s:\n", indent, enum_decl->enum_idnts[i]->svalue);
		print_expr(enum_decl->enum_idnt_values[i], frmt("   %s", indent));
	}
}

void print_union_decl(UnionDecl* union_decl, const char* indent)
{
	printf(BOLDMAGENTA);
	printf("%sunion-decl: %s{mmbrs: %d}\n", indent, union_decl->union_name, sbuffer_len(union_decl->union_mmbrs));
	printf(RESET);
	for (int i = 0; i < sbuffer_len(union_decl->union_mmbrs); i++)
		print_type_var(union_decl->union_mmbrs[i], frmt("   %s", indent));
}

void print_struct_decl(StructDecl* struct_decl, const char* indent)
{
	printf(BOLDMAGENTA);
	printf("%sstruct-decl: %s{mmbrs: %d}\n", indent, struct_decl->struct_name, sbuffer_len(struct_decl->struct_mmbrs));
	printf(RESET);
	for (int i = 0; i < sbuffer_len(struct_decl->struct_mmbrs); i++)
		print_type_var(struct_decl->struct_mmbrs[i], frmt("   %s", indent));
}

void print_type_decl(TypeDecl* type_decl, const char* indent)
{
	switch (type_decl->type)
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
		assert(0);
	}
}

void print_do_loop(DoLoop* do_loop, const char* indent)
{
	printf(BOLDMAGENTA);
	printf("%sdo-loop:\n", indent);
	indent = frmt("   %s", indent);
	printf(RESET);

	printf(BOLDCYAN);
	printf("%sdo-body:\n", indent);
	printf(RESET);
	print_block(do_loop->do_body, frmt("   %s", indent));

	printf(BOLDCYAN);
	printf("%sdo-cond:\n", indent);
	printf(RESET);
	print_expr(do_loop->do_cond, indent);
}

void print_for_loop(ForLoop* for_loop, const char* indent)
{
	printf(BOLDMAGENTA);
	printf("%sfor-loop:\n", indent);
	indent = frmt("   %s", indent);
	printf(BOLDCYAN);
	printf("%sfor-init:\n", indent);
	printf(RESET);
	print_var_decl(for_loop->for_init, frmt("   %s", indent));

	printf(BOLDCYAN);
	printf("%sfor-cond:\n", indent);
	printf(RESET);
	print_expr(for_loop->for_cond, indent);

	printf(BOLDCYAN);
	printf("%sfor-step:\n", indent);
	printf(RESET);
	print_expr(for_loop->for_step, indent);

	printf(BOLDCYAN);
	printf("%sfor-body:\n", indent);
	printf(RESET);
	print_block(for_loop->for_body, frmt("   %s", indent));
}

void print_while_loop(WhileLoop* while_loop, const char* indent)
{
	printf(BOLDMAGENTA);
	printf("%swhile-loop:\n", indent);
	indent = frmt("   %s", indent);
	printf(RESET);

	printf(BOLDCYAN);
	printf("%swhile-cond:\n", indent);
	printf(RESET);
	print_expr(while_loop->while_cond, indent);

	printf(BOLDCYAN);
	printf("%swhile-body:\n", indent);
	printf(RESET);
	print_block(while_loop->while_body, frmt("   %s", indent));
}

void print_loop_stmt(LoopStmt* loop_stmt, const char* indent)
{
	switch (loop_stmt->type)
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
		assert(0);
	}
}

void print_expr_stmt(ExprStmt* expr_stmt, const char* indent)
{
	printf(CYAN);
	printf("%sexpr-stmt:\n", indent);
	printf(RESET);
	print_expr(expr_stmt->expr, indent);
}

void print_empty_stmt(EmptyStmt* empty_stmt, const char* indent)
{
	printf(BOLDBLUE);
	printf("%sempty-stmt\n", indent);
	printf(RESET);
}

void print_if_stmt(IfStmt* if_stmt, const char* indent)
{
	printf(BOLDMAGENTA);
	printf("%sif-stmt:\n", indent);
	char* indent2 = frmt("   %s", indent);
	printf(RESET);

	printf(BOLDCYAN);
	printf("%sif-cond:\n", indent2);
	printf(RESET);
	print_expr(if_stmt->if_cond, indent2);

	printf(BOLDCYAN);
	printf("%sif-body:\n", indent2);
	printf(RESET);
	print_block(if_stmt->if_body, frmt("   %s", indent2));

	for (int i = 0; i < sbuffer_len(if_stmt->elifs); i++)
	{
		printf(BOLDMAGENTA);
		printf("%selif-stmt:\n", indent);
		printf(RESET);

		printf(BOLDCYAN);
		printf("%selif-cond:\n", indent2);
		printf(RESET);
		print_expr(if_stmt->elifs[i]->elif_cond, indent2);

		printf(BOLDCYAN);
		printf("%selif-body:\n", indent2);
		printf(RESET);
		print_block(if_stmt->elifs[i]->elif_body, frmt("   %s", indent2));
	}

	if (if_stmt->else_body)
	{
		printf(BOLDMAGENTA);
		printf("%selse-stmt:\n", indent);
		printf(RESET);
		print_block(if_stmt->else_body, frmt("   %s", indent));
	}
}

void print_jump_stmt(JumpStmt* jump_stmt, const char* indent)
{
	printf(MAGENTA);
	printf(indent);
	switch (jump_stmt->type)
	{
	case JUMP_GOTO:
		printf("goto-stmt:\n");
		print_expr(jump_stmt->additional_expr, indent);
		break;
	case JUMP_BREAK:
		printf("break-stmt\n");
		break;
	case JUMP_RETURN:
		printf("return-stmt\n");
		if (jump_stmt->additional_expr)
			print_expr(jump_stmt->additional_expr, indent);
		break;
	case JUMP_CONTINUE:
		printf("continue-stmt\n");
		break;
	default:
		assert(0);
	}
	printf(RESET);
}

void print_case_stmt(Case* case_stmt, const char* indent)
{
	printf(BOLDMAGENTA);
	printf("%scase-stmt:\n", indent);
	indent = frmt("%s   ", indent);
	printf("%scase-value:\n", indent);
	printf(RESET);
	print_expr(case_stmt->case_value, indent);

	if (case_stmt->case_body)
	{
		printf(BOLDMAGENTA);
		printf("%scase-body:\n", indent);
		printf(RESET);
		print_block(case_stmt->case_body, frmt("%s   ", indent));
	}
}

void print_switch_stmt(SwitchStmt* switch_stmt, const char* indent)
{
	printf(BOLDMAGENTA);
	printf("%sswitch-stmt:\n", indent);
	indent = frmt("%s   ", indent);
	printf(RESET);

	printf(BOLDMAGENTA);
	printf("%sswitch-cond:\n", indent);
	printf(RESET);
	print_expr(switch_stmt->switch_cond, indent);

	for (int i = 0; i < sbuffer_len(switch_stmt->switch_cases); i++)
		print_case_stmt(switch_stmt->switch_cases[i], frmt("%s   ", indent));

	if (switch_stmt->switch_default)
	{
		printf(BOLDMAGENTA);
		printf("%sdefault-stmt:\n", indent);
		printf(RESET);
		print_block(switch_stmt->switch_default, frmt("%s   ", indent));
	}
}

void print_import_stmt(ImportStmt* import_stmt, const char* indent)
{
	printf(BOLDRED);
	printf("%simport-stmt:\n", indent);
	printf(RESET);
	for (int i = 0; i < sbuffer_len(import_stmt->imported_ast->stmts); i++)
		print_stmt(import_stmt->imported_ast->stmts[i], indent);
}

void print_stmt(Stmt* stmt, const char* indent)
{
	char* new_indent = frmt("%s   ", indent);
	if (stmt)
		switch (stmt->type)
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
			assert(0);
		}
	else
		printf("%s   no-body\n", indent);
}