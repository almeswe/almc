#include "ast-printer.h"

#define TREE_BR 	"\\_"
#define TREE_BV 	"| "
#define TREE_BVR    "|_"

void print_ast(AstRoot* ast) {
	printf(_c(BCYN, "%s-"), "main");
	print_ast_root(ast, "");
}

void print_ast_root(AstRoot* ast, const char* indent) {
	print_ast_header(ast); printf("\n");
	const char* new_indent = frmt("%s   ", indent);
	const size_t size = sbuffer_len(ast->stmts);
	for (size_t i = 0; i < size; i++) {
		printf("%s", new_indent);
		print_stmt(ast->stmts[i], new_indent);
		if (i < (size-1)) {
			printf("%s", "\n");
		}
	}
}

void print_ast_header(AstRoot* ast) {
	char buffer[512];
	printf(_c(BCYN, "%s"), "ast-root "); 
	for (size_t i = 0; i < sbuffer_len(ast->from); i++) {
		const char* filename = basename(ast->from[i]);
		const char* abspath  = get_curr_dir();
		if (i == 0) {
			printf("%s", "<");
		}
		if (abspath[strlen(abspath)-1] != PATH_SEPARATOR) {
			printf("from=\"" _c(BGRN, "%s/") _c(BYEL, "%s") "\"", abspath, filename);
		} else {
			printf("from=\"" _c(BGRN, "%s") _c(BYEL, "%s") "\"", abspath, filename);
		}
		if (i+1 < sbuffer_len(ast->from)) {
			printf("%s", " ");
		}
	}
	printf("%s", ">");
}

void print_type(Type* type) {
	if (type == NULL || type->kind == TYPE_UNKNOWN) {
		return printf("%s", "type=" _c(RED, "?")), (void)0;
	}
	printf("type=" _c(BBLU, "%s") "(" _c(BBLU, "%lu") ")",
		type_tostr_plain(type), type->size);
	return;
	if (type == NULL || type->kind == TYPE_UNKNOWN) {
		return printf("%s", "type=<" _c(RED, "?") ">"), (void)0;
	}
	printf("type=<bytes=" _c(WHT, "%lu") ", " _c(BBLU, "%s") ">",
		type->size, type_tostr_plain(type));
}

void print_idnt(Idnt* idnt) {
	printf(_c(BCYN, "idnt") " <var=" _c(BYEL, "%s") ", ", idnt->svalue);
	print_type(idnt->type);
	if (idnt->attrs.is_enum_member) {
		printf(", attrs={enum_member=%d}", idnt->attrs.is_enum_member);
	}
	printf("%s\n", ">");
}

char get_explicit_escapec(char escapec) {
	switch (escapec) {
		case '\a': 	return 'a';
		case '\b': 	return 'b';
		case '\f': 	return 'f';
		case '\n': 	return 'n';
		case '\r': 	return 'r';
		case '\t': 	return 't';
		case '\v': 	return 'v';
		case '\0': 	return '0';
		case '\\': 	return '\\';
		case '\'':	return '\'';
		case '\"':	return '\"';
		default:
			report_error("Unknown control character met in get_explicit_escapec()", NULL);
	}
}

void print_const(Const* cnst) {
	printf(_c(BCYN, "const") " <val=");
	switch (cnst->kind) {
		case CONST_CHAR: 	
			if (!iscntrl((char)cnst->ivalue)) {
				printf("\'" _c(BYEL, "%c") "\'(%ld), ", (char)cnst->ivalue, cnst->ivalue);
			} else {
				printf("\'" _c(BYEL, "\\%c") "\'(%ld), ", 
					get_explicit_escapec((char)cnst->ivalue), cnst->ivalue);
			}
			break;
		case CONST_INT:		_b(printf(_c(BYEL, "%ld") ", ", cnst->ivalue));
		case CONST_UINT:	_b(printf(_c(BYEL, "%lu") ", ", cnst->uvalue));
		case CONST_FLOAT:	_b(printf(_c(BYEL "%.10f")", ", cnst->fvalue));
		default:
			report_error("Unknown const kind met in print_const()\n", NULL);
	}
	print_type(cnst->type);
	printf("%s\n", ">");
}

void print_str(Str* str) {
	printf("%s", _c(BCYN, "const") " <val=\"" BYEL);
	for (size_t i = 0; i < strlen(str->svalue); i++) {
		char curr_char = str->svalue[i]; 
		if (iscntrl(curr_char)) {
			putc('\\', stdout), putc(get_explicit_escapec(curr_char), stdout);
		} else {
			putc(curr_char, stdout);
		}
	}
	printf("%s", RESET "\", ");
	print_type(str->type);
	printf("%s\n", ">");
}

void print_call(FuncCall* call, const char* indent) {
	printf(_c(BCYN, "fcall") " <f=" _c(BYEL, "%s") ", argc=%lu, ret-", 
		call->name->value, sbuffer_len(call->args));
	print_type(call->type);
	printf(">\n");
	const char* new_indent = frmt("%s   ", indent);
	for (size_t i = 0; i < sbuffer_len(call->args); i++) {
		printf("%s" TREE_BR "arg<%lu> = ", new_indent, i+1);
		print_expr(call->args[i], new_indent);
	}
}

void print_call2(FuncCall2* call, const char* indent) {
	printf(_c(BGRN, "fcall2") ", argc=%lu, ret-", sbuffer_len(call->args));
	print_type(call->type);
	printf(">\n");
	const char* new_indent = frmt("%s   ", indent);
	printf("%s" TREE_BR "rexpr = ", new_indent);
	print_expr(call->rexpr, new_indent);
	for (size_t i = 0; i < sbuffer_len(call->args); i++) {
		printf("%s" TREE_BR "arg<%lu> = ", new_indent, i+1);
		print_expr(call->args[i], new_indent);
	}
}

void print_unary_expr(UnaryExpr* unary_expr, const char* indent) {
	const char* unary_str[] = {
		"+ (plus)", "- (minus)", "& (address)",
		"* (deref)", "! (not)", "~ (bw-not)",
		"cast",
		"sizeof",
		"lengthof",
	};

	printf(_c(BGRN, "unary") " <" _c(BYEL, "%s") ", ", unary_str[unary_expr->kind]);
	print_type(unary_expr->type);
	if (unary_expr->cast_type != NULL) {
		printf(", %s-", unary_str[unary_expr->kind]);
		print_type(unary_expr->cast_type);
	}
	printf("%s\n", ">");
	const char* new_indent = frmt("%s   ", indent);
	printf("%s" TREE_BR "expr = ", new_indent);
	print_expr(unary_expr->expr, new_indent);
}

void print_binary_expr(BinaryExpr* binary_expr, const char* indent) {
	const char* binary_str[] = {
		"+ (add)",
		"- (sub)",
		"/ (div)",
		"% (mod)",
		"* (mul)",

		", (comma)",
		"<< (shl)",
		">> (shr)",

		"< (lessthan)",
		"> (greaterthan)",
		"<= (lesseqthan)",
		">= (greatereqthan)",

		"|| (or)",
		"&& (and)",
		"== (eq)",
		"!= (neq)",

		"| (bw-or)",
		"& (bw-and)",
		"^ (bw-xor)",

		"=  (asgn)",
		"+= (add-asgn)",
		"-= (sub-asgn)",
		"*= (mul-asgn)",
		"/= (div-asgn)",
		"%= (mod-asgn)",
		"<<= (shl-asgn)",
		">>= (shr-asgn)",
		"|= (bw-or-asgn)",
		"&= (bw-and-asgn)",
		"^= (bw-xor-asgn)",

		". (accessor)",
		"-> (ptr-accessor)",
		"[] (arr-accessor)"
	};

	printf(_c(BGRN, "binary") " <" _c(BYEL, "%s") ", ", binary_str[binary_expr->kind]);
	print_type(binary_expr->type);
	printf("%s\n", ">");
	const char* new_indent = frmt("%s   ", indent);
	printf("%s" TREE_BR "lexpr = ", new_indent);
	print_expr(binary_expr->lexpr, new_indent);
	printf("%s" TREE_BR "rexpr = ", new_indent);
	print_expr(binary_expr->rexpr, new_indent);
}

void print_ternary_expr(TernaryExpr* ternary_expr, const char* indent) {
	printf(_c(BGRN, "ternary") " <");
	print_type(ternary_expr->type);
	printf("%s\n", ">");
	const char* new_indent = frmt("%s   ", indent);
	printf("%s" TREE_BR "cexpr = ", new_indent);
	print_expr(ternary_expr->cond, new_indent);
	printf("\n%s" TREE_BR "lexpr = ", new_indent);
	print_expr(ternary_expr->lexpr, new_indent);
	printf("\n%s" TREE_BR "rexpr = ", new_indent);
	print_expr(ternary_expr->rexpr, new_indent);
}

void print_initializer_expr(Initializer* init_expr, const char* indent) {
	printf(_c(BGRN, "initializer") " <valc=%lu, ", sbuffer_len(init_expr->values));
	print_type(init_expr->type);
	printf("%s\n", ">");
	const char* new_indent = frmt("%s   ", indent);
	for (size_t i = 0; i < sbuffer_len(init_expr->values); i++) {
		printf("%s" TREE_BR "val<%lu> = ", new_indent, i+1);
		print_expr(init_expr->values[i], new_indent);
	}
}

void print_expr(Expr* expr, const char* indent) {
	if (expr == NULL) {
		printf(_c(BRED, "%s\n"), "NULL");
	}
	else {
		switch (expr->kind) {
			case EXPR_IDNT:			_b(print_idnt(expr->idnt));
			case EXPR_CONST:		_b(print_const(expr->cnst));
			case EXPR_STRING:		_b(print_str(expr->str));
			case EXPR_FUNC_CALL:	_b(print_call(expr->func_call, indent));
			case EXPR_FUNC_CALL2:	_b(print_call2(expr->func_call2, indent));
			case EXPR_UNARY_EXPR:	_b(print_unary_expr(expr->unary_expr, indent));
			case EXPR_BINARY_EXPR:  _b(print_binary_expr(expr->binary_expr, indent));
			case EXPR_TERNARY_EXPR: _b(print_ternary_expr(expr->ternary_expr, indent));
			case EXPR_INITIALIZER:	_b(print_initializer_expr(expr->initializer, indent));
			default:
				report_error(frmt("Unknown expression kind met"
					" in function: %s", __FUNCTION__), NULL);
		}
	}
}

void print_block_stmt(Block* block, const char* indent) {
	if (block == NULL) {
		printf(_c(BMAG, "scope") " <c=%lu>\n", 0ul);
		return;
	}
	printf(_c(BMAG, "scope") " <c=%lu>\n", sbuffer_len(block->stmts));
	const char* new_indent = frmt("%s   ", indent);
	for (size_t i = 0; i < sbuffer_len(block->stmts); i++) {
		printf("%s" TREE_BR "<%ld> = ", new_indent, i+1);
		print_stmt(block->stmts[i], new_indent);
	}
}

void print_var_decl_stmt(VarDecl* var_decl_stmt, const char* indent) {
	if (var_decl_stmt->is_auto) {
		printf(_c(BMAG, "%s"), "var-stmt");	
	}
	else {
		printf(_c(BMAG, "%s"), "let-stmt");	
	}
	printf(" <var=" _c(BYEL ,"%s") ", ", var_decl_stmt->type_var->var);
	print_type(var_decl_stmt->type_var->type);
	printf("%s\n", ">");
	const char* new_indent = frmt("%s   ", indent);
	printf("%s" TREE_BR "expr_init = ", new_indent);
	print_expr(var_decl_stmt->var_init, new_indent);
}

void print_struct_member(Member* member, const char* struct_name) {
	printf("<name=" _c(BYEL, "%s") ", %s" _c(BYEL, "+%d") ", ", 
		member->name, struct_name, member->offset);
	print_type(member->type);
	printf("%s\n", ">");
}

void print_enum(EnumDecl* enum_decl, const char* indent) {
	printf(_c(BMAG, "enum") " <name=" _c(BYEL, "%s") ", memberc=%lu>\n", 
		enum_decl->name->value, sbuffer_len(enum_decl->members));	
	const char* new_indent = frmt("%s   ", indent);
	for (size_t i = 0; i < sbuffer_len(enum_decl->members); i++) {
		printf("%s" TREE_BR "<%s> = " _c(BYEL, "%ld") " <", new_indent, enum_decl->members[i]->name, 
			evaluate_expr_itype(enum_decl->members[i]->value));
		print_type(retrieve_expr_type(enum_decl->members[i]->value));
		printf("%s\n", ">");
	}
}

void print_struct(StructDecl* struct_decl, const char* indent) {
	printf(_c(BMAG, "struct") " <name=" _c(BYEL, "%s") ", memberc=%lu>\n", 
		struct_decl->name->value, sbuffer_len(struct_decl->members));	
	const char* new_indent = frmt("%s   ", indent);
	for (size_t i = 0; i < sbuffer_len(struct_decl->members); i++) {
		printf("%s" TREE_BR "<%lu> = ", new_indent, i+1); 
		print_struct_member(struct_decl->members[i], struct_decl->name->value);
	}
}

void print_union(UnionDecl* union_decl, const char* indent) {
	printf(_c(BMAG, "union") " <name=" _c(BYEL, "%s") ", memberc=%lu>\n", 
		union_decl->name->value, sbuffer_len(union_decl->members));	
	const char* new_indent = frmt("%s   ", indent);
	for (size_t i = 0; i < sbuffer_len(union_decl->members); i++) {
		printf("%s" TREE_BR "<%lu> = ", new_indent, i+1);
		print_struct_member(union_decl->members[i], union_decl->name->value);
	}
}

void print_type_decl_stmt(TypeDecl* type_decl_stmt, const char* indent) {
	switch (type_decl_stmt->kind) {
		case TYPE_DECL_ENUM: 	_b(print_enum(type_decl_stmt->enum_decl, indent));
		case TYPE_DECL_STRUCT: 	_b(print_struct(type_decl_stmt->struct_decl, indent));
		case TYPE_DECL_UNION:	_b(print_union(type_decl_stmt->union_decl, indent));
		default:
			report_error(frmt("Unknown type declaration kind met"
				" in function: %s", __FUNCTION__), NULL);
	}
}

void print_func_decl_stmt(FuncDecl* func_decl, const char* indent) {
	printf(_c(BMAG, "fnc") " <name=" _c(BYEL, "%s") ", paramc=%lu, ", 
		func_decl->name->value, sbuffer_len(func_decl->params));
	if (func_decl->spec->is_entry | func_decl->spec->is_external | func_decl->spec->is_vararg) {
		printf("%s", "specs={");
		if (func_decl->spec->is_entry) {
			printf("%s", "entry ");
		}
		if (func_decl->spec->is_external) {
			printf("%s", "external ");
		}
		if (func_decl->spec->is_vararg) {
			printf("%s", "vararg");
		}
		printf("%s", "}, ");
	}
	printf(_c(BYEL, "%s")", ", func_decl->conv->repr); 
	printf("%s", "ret-");
	print_type(func_decl->type->attrs.func.ret); printf("%s\n", ">");
	const char* new_indent = frmt("%s   ", indent);
	for (size_t i = 0; i < sbuffer_len(func_decl->params); i++) {
		printf("%s" TREE_BR " param<%lu> = <name=" _c(BYEL, "%s") ", ", 
			new_indent, i+1, func_decl->params[i]->var);
		print_type(func_decl->params[i]->type);
		printf("%s", "\n");
	}
	printf("%s" TREE_BR "body = ", new_indent);
	print_block_stmt(func_decl->body, new_indent);
}

void print_elif_stmt(ElseIf* elif_stmt, const char* indent) {
	printf(_c(BMAG, "%s\n"), "elseif");
	const char* new_indent = frmt("%s   ", indent);
	printf("%s" TREE_BR "condition = ", new_indent); 
	print_expr(elif_stmt->cond, new_indent);
	printf("%s" TREE_BR "body = ", new_indent); 
	print_block_stmt(elif_stmt->body, new_indent);
}

void print_if_stmt(IfStmt* if_stmt, const char* indent) {
	printf(_c(BMAG, "if") " <else ifs=" _c(BYEL, "%lu") ", else=%u>\n", 
		sbuffer_len(if_stmt->elifs), (unsigned)(if_stmt->else_body != NULL));
	const char* new_indent = frmt("%s   ", indent);
	printf("%s" TREE_BR "condition = ", new_indent); 
	print_expr(if_stmt->cond, new_indent);
	printf("%s" TREE_BR "body = ", new_indent); 
	print_block_stmt(if_stmt->body, new_indent);
	for (size_t i = 0; i < sbuffer_len(if_stmt->elifs); i++) {
		printf("%s" TREE_BR, new_indent);
		print_elif_stmt(if_stmt->elifs[i], new_indent);
	}
	if (if_stmt->else_body != NULL) {
		printf("%s" TREE_BR _c(BMAG, "else") " = ", new_indent);
		print_block_stmt(if_stmt->else_body, new_indent);
	}
}

void print_expr_stmt(ExprStmt* expr_stmt, const char* indent) {
	printf(_c(BMAG, "%s") " = ", "exprstmt");
	const char* new_indent = frmt("%s   ", indent);
	print_expr(expr_stmt->expr, indent);
}

void print_break_stmt(const char* indent) {
	printf(_c(BRED, "%s"), "break\n");
}

void print_continue_stmt(const char* indent) {
	printf(_c(BRED, "%s"), "continue\n");
}

void print_goto_stmt(JumpStmt* jump_stmt, const char* indent) {
	printf(_c(BRED, "goto") " <label=" _c(BYEL, "%s") ">\n", 
		jump_stmt->expr->idnt->svalue);
}

void print_return_stmt(JumpStmt* jump_stmt, const char* indent) {
	printf(_c(BRED, "%s"), "return\n");
	const char* new_indent = frmt("%s   ", indent);
	printf("%s" TREE_BR "value = ", new_indent); 
	print_expr(jump_stmt->expr, new_indent);
}

void print_jump_stmt(JumpStmt* jump_stmt, const char* indent) {
	switch (jump_stmt->kind) {
		case JUMP_GOTO: 		_b(print_goto_stmt(jump_stmt, indent));
		case JUMP_BREAK: 		_b(print_break_stmt(indent));
		case JUMP_RETURN: 		_b(print_return_stmt(jump_stmt, indent));
		case JUMP_CONTINUE: 	_b(print_continue_stmt(indent));
		default:
			report_error(frmt("Unknown loop kind met"
				" in function: %s", __FUNCTION__), NULL);
	}
}

void print_while_loop_stmt(WhileLoop* while_stmt, const char* indent) {
	printf(_c(BMAG, "%s"), "while-loop\n");
	const char* new_indent = frmt("%s   ", indent);
	printf("%s" TREE_BR "condition = ", new_indent); 
	print_expr(while_stmt->cond, new_indent);
	printf("%s" TREE_BR "body = ", new_indent); 
	print_block_stmt(while_stmt->body, new_indent);
}

void print_for_loop_stmt(ForLoop* for_stmt, const char* indent) {
	printf(_c(BMAG, "%s"), "for-loop\n");
	const char* new_indent = frmt("%s   ", indent);
	printf("%s" TREE_BR "init = ", new_indent); 
	print_var_decl_stmt(for_stmt->init, new_indent);
	printf("%s" TREE_BR "condition = ", new_indent); 
	print_expr(for_stmt->cond, new_indent);
	printf("%s" TREE_BR "step = ", new_indent); 
	print_expr(for_stmt->step, new_indent);
	printf("%s" TREE_BR "body = ", new_indent); 
	print_block_stmt(for_stmt->body, new_indent);
}

void print_do_loop_stmt(DoLoop* do_stmt, const char* indent) {
	printf(_c(BMAG, "%s"), "do-while-loop\n");
	const char* new_indent = frmt("%s   ", indent);
	printf("%s" TREE_BR "body = ", new_indent); 
	print_block_stmt(do_stmt->body, new_indent);
	printf("%s" TREE_BR "condition = ", new_indent); 
	print_expr(do_stmt->cond, new_indent);
}

void print_loop_stmt(LoopStmt* loop_stmt, const char* indent) {
	switch (loop_stmt->kind) {
		case LOOP_DO: 		_b(print_do_loop_stmt(loop_stmt->do_loop, indent));
		case LOOP_FOR: 		_b(print_for_loop_stmt(loop_stmt->for_loop, indent));
		case LOOP_WHILE: 	_b(print_while_loop_stmt(loop_stmt->while_loop, indent));
		default:
			report_error(frmt("Unknown loop kind met"
				" in function: %s", __FUNCTION__), NULL);
	}
}

void print_label_decl_stmt(LabelDecl* label_decl, const char* indent) {
	printf(_c(BMAG, "label") " <l=" _c(BYEL, "%s") ">\n", label_decl->name->value);
}

void print_switch_case_stmt(SwitchStmt* switch_stmt, const char* indent) {
	printf(_c(BMAG, "switch-case") " <cases=" _c(BYEL, "%lu") ", default=" _c(BYEL, "%d") ">\n",
		sbuffer_len(switch_stmt->cases), switch_stmt->default_case != NULL);
	const char* new_indent = frmt("%s   ", indent);
	printf("%s" TREE_BR "condition = ", new_indent); 
	print_expr(switch_stmt->cond, new_indent);
	for (size_t i = 0; i < sbuffer_len(switch_stmt->cases); i++) {
		printf("%s" TREE_BR "case " _c(BYEL, "%ld"), new_indent, 
			evaluate_expr_itype(switch_stmt->cases[i]->value));
		if (!switch_stmt->cases[i]->is_conjucted) {
			printf("%s",  " = ");
			print_block_stmt(switch_stmt->cases[i]->body, indent);
		}
	}
	if (switch_stmt->default_case != NULL) {
		printf("%s" TREE_BR "default-case = ", new_indent);
		print_block_stmt(switch_stmt->default_case, new_indent);
	}
}

void print_import_stmt(ImportStmt* import_stmt, const char* indent) {
	printf(_c(BCYN, "%s-"), "imported");
	print_ast_root(import_stmt->ast, indent);
}

void print_stmt(Stmt* stmt, const char* indent) {
	if (stmt == NULL) {
		printf(_c(BRED, "%s"), "NULL");
	}
	else {
		switch (stmt->kind) {
			case STMT_IF:			_b(print_if_stmt(stmt->if_stmt, indent));
			case STMT_LOOP:			_b(print_loop_stmt(stmt->loop_stmt, indent));
			case STMT_EXPR:			_b(print_expr_stmt(stmt->expr_stmt, indent));
			case STMT_JUMP:			_b(print_jump_stmt(stmt->jump_stmt, indent));
			case STMT_BLOCK:		_b(print_block_stmt(stmt->block, indent));
			case STMT_EMPTY:		_b(;);
			case STMT_IMPORT:		_b(print_import_stmt(stmt->import_stmt, indent));
			case STMT_SWITCH:		_b(print_switch_case_stmt(stmt->switch_stmt, indent));
			case STMT_VAR_DECL:		_b(print_var_decl_stmt(stmt->var_decl, indent));
			case STMT_TYPE_DECL:	_b(print_type_decl_stmt(stmt->type_decl, indent));
			case STMT_FUNC_DECL:	_b(print_func_decl_stmt(stmt->func_decl, indent));
			case STMT_LABEL_DECL:   _b(print_label_decl_stmt(stmt->label_decl, indent));
			default:
				report_error(frmt("Unknown statement kind met"
					" in function: %s", __FUNCTION__), NULL);
		}
	}
}