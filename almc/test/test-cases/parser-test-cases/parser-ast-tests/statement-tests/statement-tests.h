#ifndef ALMC_GLOBAL_AST_STATEMENT_TEST
#define ALMC_GLOBAL_AST_STATEMENT_TEST

inline void test_func_decl_stmts()
{
	Lexer* lexer = lexer_new(
		"test\\test-cases\\parser-test-cases\\parser-ast-tests\\statement-tests\\declaration_stmts\\func_decl.almc", FROM_FILE);
	char* path_copy = _strdup(lexer->curr_file);
	Parser* parser = parser_new(path_copy, lex(lexer));
	AstRoot* ast = parse(parser);

	assert(sbuffer_len(ast->stmts) == 4);
	assert(ast->stmts[0]->kind == STMT_FUNC_DECL);
	assert(strcmp(ast->stmts[0]->func_decl->name->svalue, "a_1") == 0);
	assert(!ast->stmts[0]->func_decl->body);
	assert(sbuffer_len(ast->stmts[0]->func_decl->params) == 0);
	assert(IS_VOID_TYPE(ast->stmts[0]->func_decl->type));

	assert(ast->stmts[1]->kind == STMT_FUNC_DECL);
	assert(strcmp(ast->stmts[1]->func_decl->name->svalue, "a_2") == 0);
	assert(!ast->stmts[1]->func_decl->body);
	assert(strcmp(ast->stmts[1]->func_decl->params[0]->type->repr, I32_TYPE) == 0);
	assert(strcmp(ast->stmts[1]->func_decl->params[1]->var, "b") == 0);
	assert(strcmp(ast->stmts[1]->func_decl->type->repr, I32_TYPE) == 0);

	assert(ast->stmts[2]->kind == STMT_FUNC_DECL);
	assert(strcmp(ast->stmts[2]->func_decl->name->svalue, "a_3") == 0);
	assert(ast->stmts[2]->func_decl->spec->is_entry);
	assert(!ast->stmts[2]->func_decl->body);
	assert(sbuffer_len(ast->stmts[2]->func_decl->params) == 0);
	assert(IS_VOID_TYPE(ast->stmts[2]->func_decl->type));

	assert(ast->stmts[3]->kind == STMT_FUNC_DECL);
	assert(strcmp(ast->stmts[3]->func_decl->name->svalue, "a_4") == 0);
	assert(!ast->stmts[3]->func_decl->body);
	assert(sbuffer_len(ast->stmts[3]->func_decl->params) == 0);
	assert(IS_VOID_TYPE(ast->stmts[3]->func_decl->type));

	free(path_copy);
	lexer_free(lexer);
	parser_free(parser);
	clear_imported_modules();
	ast_free(ast);
}

inline void test_type_decl_stmts()
{
	Lexer* lexer = lexer_new(
		"test\\test-cases\\parser-test-cases\\parser-ast-tests\\statement-tests\\declaration_stmts\\type_decl.almc", FROM_FILE);
	char* path_copy = _strdup(lexer->curr_file);
	Parser* parser = parser_new(path_copy, lex(lexer));
	AstRoot* ast = parse(parser);

	assert(sbuffer_len(ast->stmts) == 11);
	assert(ast->stmts[0]->type_decl->kind == TYPE_DECL_STRUCT);
	assert(strcmp(ast->stmts[0]->type_decl->struct_decl->name, "a") == 0);
	assert(!ast->stmts[0]->type_decl->struct_decl->members);

	assert(ast->stmts[1]->type_decl->kind == TYPE_DECL_STRUCT);
	assert(strcmp(ast->stmts[1]->type_decl->struct_decl->name, "a") == 0);
	assert(strcmp(ast->stmts[1]->type_decl->struct_decl->members[0]->name, "noname") == 0);

	assert(ast->stmts[2]->type_decl->kind == TYPE_DECL_STRUCT);
	assert(strcmp(ast->stmts[2]->type_decl->struct_decl->name, "a") == 0);
	assert(strcmp(ast->stmts[2]->type_decl->struct_decl->members[0]->name, "a_m") == 0);
	assert(strcmp(ast->stmts[2]->type_decl->struct_decl->members[1]->type->repr, I16_TYPE) == 0);

	assert(ast->stmts[3]->type_decl->kind == TYPE_DECL_ENUM);
	assert(strcmp(ast->stmts[3]->type_decl->enum_decl->name, "a_e") == 0);
	assert(!ast->stmts[3]->type_decl->enum_decl->members);

	assert(ast->stmts[4]->type_decl->kind == TYPE_DECL_ENUM);
	assert(strcmp(ast->stmts[4]->type_decl->enum_decl->name, "") == 0);
	assert(ast->stmts[4]->type_decl->enum_decl->members[0]->value->cnst->ivalue == 255);

	assert(ast->stmts[5]->type_decl->kind == TYPE_DECL_ENUM);
	assert(strcmp(ast->stmts[5]->type_decl->enum_decl->name, "a") == 0);
	assert(ast->stmts[5]->type_decl->enum_decl->members[0]->value->cnst->ivalue == 0);
	assert(ast->stmts[5]->type_decl->enum_decl->members[1]->value->cnst->ivalue == 1);
	assert(ast->stmts[5]->type_decl->enum_decl->members[2]->value->cnst->ivalue == 2);
	assert(strcmp(ast->stmts[5]->type_decl->enum_decl->members[1]->name, "s") == 0);

	assert(ast->stmts[6]->type_decl->kind == TYPE_DECL_ENUM);
	assert(strcmp(ast->stmts[6]->type_decl->enum_decl->name, "b") == 0);
	assert(ast->stmts[6]->type_decl->enum_decl->members[0]->value->cnst->ivalue == 0xf);
	assert(ast->stmts[6]->type_decl->enum_decl->members[1]->value->cnst->ivalue == (0xf + 1));
	assert(ast->stmts[6]->type_decl->enum_decl->members[2]->value->cnst->ivalue == 1);
	assert(ast->stmts[6]->type_decl->enum_decl->members[3]->value->cnst->ivalue == 2);
	assert(strcmp(ast->stmts[6]->type_decl->enum_decl->members[3]->name, "g") == 0);

	assert(ast->stmts[7]->type_decl->kind == TYPE_DECL_ENUM);
	assert(strcmp(ast->stmts[7]->type_decl->enum_decl->name, "c") == 0);
	assert(ast->stmts[7]->type_decl->enum_decl->members[0]->value->kind == EXPR_BINARY_EXPR);
	assert(ast->stmts[7]->type_decl->enum_decl->members[1]->value->binary_expr->rexpr->cnst->ivalue == 1);
	assert(ast->stmts[7]->type_decl->enum_decl->members[2]->value->cnst->ivalue == 0xf);
	assert(ast->stmts[7]->type_decl->enum_decl->members[3]->value->cnst->ivalue == (0xf + 1));
	assert(strcmp(ast->stmts[7]->type_decl->enum_decl->members[2]->name, "t") == 0);

	assert(ast->stmts[8]->type_decl->kind == TYPE_DECL_UNION);
	assert(strcmp(ast->stmts[8]->type_decl->union_decl->name, "a") == 0);
	assert(!ast->stmts[8]->type_decl->union_decl->members);

	assert(ast->stmts[9]->type_decl->kind == TYPE_DECL_UNION);
	assert(strcmp(ast->stmts[9]->type_decl->union_decl->name, "a") == 0);
	assert(strcmp(ast->stmts[9]->type_decl->union_decl->members[0]->type->repr, I32_TYPE) == 0);

	assert(ast->stmts[10]->type_decl->kind == TYPE_DECL_UNION);
	assert(strcmp(ast->stmts[10]->type_decl->union_decl->name, "b_u") == 0);
	assert(strcmp(ast->stmts[10]->type_decl->union_decl->members[0]->type->repr, I32_TYPE) == 0);
	assert(strcmp(ast->stmts[10]->type_decl->union_decl->members[1]->name, "c") == 0);

	free(path_copy);
	lexer_free(lexer);
	parser_free(parser);
	clear_imported_modules();
	ast_free(ast);
}

inline void test_var_decl_stmts()
{
	Lexer* lexer = lexer_new(
		"test\\test-cases\\parser-test-cases\\parser-ast-tests\\statement-tests\\declaration_stmts\\var_decl.almc", FROM_FILE);
	char* path_copy = _strdup(lexer->curr_file);
	Parser* parser = parser_new(path_copy, lex(lexer));
	AstRoot* ast = parse(parser);

	assert(sbuffer_len(ast->stmts) == 2);
	assert(ast->stmts[0]->kind == STMT_VAR_DECL);
	assert(strcmp(ast->stmts[0]->var_decl->type_var->var, "a") == 0);
	assert(strcmp(ast->stmts[0]->var_decl->type_var->type->repr, I32_TYPE) == 0);
	assert(!ast->stmts[0]->var_decl->var_init);

	assert(ast->stmts[1]->kind == STMT_VAR_DECL);
	assert(strcmp(ast->stmts[1]->var_decl->type_var->var, "b") == 0);
	assert(strcmp(ast->stmts[1]->var_decl->type_var->type->repr, I32_TYPE) == 0);
	assert(ast->stmts[1]->var_decl->var_init->cnst->ivalue == 2);

	free(path_copy);
	lexer_free(lexer);
	parser_free(parser);
	clear_imported_modules();
	ast_free(ast);
}

inline void test_do_loop_stmts()
{
	Lexer* lexer = lexer_new(
		"test\\test-cases\\parser-test-cases\\parser-ast-tests\\statement-tests\\loop_stmts\\do_while_loop.almc", FROM_FILE);
	char* path_copy = _strdup(lexer->curr_file);
	Parser* parser = parser_new(path_copy, lex(lexer));
	AstRoot* ast = parse(parser);

	assert(sbuffer_len(ast->stmts) == 2);
	assert(ast->stmts[0]->kind == STMT_LOOP);
	assert(ast->stmts[0]->loop_stmt->kind == LOOP_DO);
	assert(ast->stmts[0]->loop_stmt->do_loop->cond->kind == EXPR_CONST);
	assert(sbuffer_len(ast->stmts[0]->loop_stmt->do_loop->body->stmts) == 1);

	assert(ast->stmts[1]->kind == STMT_LOOP);
	assert(ast->stmts[1]->loop_stmt->kind == LOOP_DO);
	assert(ast->stmts[1]->loop_stmt->do_loop->cond->kind == EXPR_CONST);
	assert(ast->stmts[1]->loop_stmt->do_loop->body->stmts[0]->kind == STMT_EMPTY);

	free(path_copy);
	lexer_free(lexer);
	parser_free(parser);
	clear_imported_modules();
	ast_free(ast);
}

inline void test_while_loop_stmts()
{
	Lexer* lexer = lexer_new(
		"test\\test-cases\\parser-test-cases\\parser-ast-tests\\statement-tests\\loop_stmts\\while_loop.almc", FROM_FILE);
	char* path_copy = _strdup(lexer->curr_file);
	Parser* parser = parser_new(path_copy, lex(lexer));
	AstRoot* ast = parse(parser);

	assert(sbuffer_len(ast->stmts) == 2);
	assert(ast->stmts[0]->kind == STMT_LOOP);
	assert(ast->stmts[0]->loop_stmt->kind == LOOP_WHILE);
	assert(ast->stmts[0]->loop_stmt->while_loop->cond->kind == EXPR_CONST);
	assert(sbuffer_len(ast->stmts[0]->loop_stmt->while_loop->body->stmts) == 1);

	assert(ast->stmts[1]->kind == STMT_LOOP);
	assert(ast->stmts[1]->loop_stmt->kind == LOOP_WHILE);
	assert(ast->stmts[1]->loop_stmt->while_loop->cond->kind == EXPR_CONST);
	assert(ast->stmts[1]->loop_stmt->while_loop->body->stmts[0]->kind == STMT_EMPTY);

	free(path_copy);
	lexer_free(lexer);
	parser_free(parser);
	clear_imported_modules();
	ast_free(ast);
}

inline void test_for_loop_stmts()
{
	Lexer* lexer = lexer_new(
		"test\\test-cases\\parser-test-cases\\parser-ast-tests\\statement-tests\\loop_stmts\\for_loop.almc", FROM_FILE);
	char* path_copy = _strdup(lexer->curr_file);
	Parser* parser = parser_new(path_copy, lex(lexer));
	AstRoot* ast = parse(parser);

	assert(sbuffer_len(ast->stmts) == 4);
	assert(ast->stmts[0]->kind == STMT_LOOP);
	assert(ast->stmts[0]->loop_stmt->kind == LOOP_FOR);
	assert(ast->stmts[0]->loop_stmt->for_loop->init);
	assert(ast->stmts[0]->loop_stmt->for_loop->cond);
	assert(ast->stmts[0]->loop_stmt->for_loop->step);
	assert(sbuffer_len(ast->stmts[0]->loop_stmt->for_loop->body->stmts) == 1);

	assert(ast->stmts[1]->kind == STMT_LOOP);
	assert(ast->stmts[1]->loop_stmt->kind == LOOP_FOR);
	assert(!ast->stmts[1]->loop_stmt->for_loop->init);
	assert(ast->stmts[1]->loop_stmt->for_loop->cond);
	assert(ast->stmts[1]->loop_stmt->for_loop->step);
	assert(sbuffer_len(ast->stmts[1]->loop_stmt->for_loop->body->stmts) == 1);

	assert(ast->stmts[2]->kind == STMT_LOOP);
	assert(ast->stmts[2]->loop_stmt->kind == LOOP_FOR);
	assert(!ast->stmts[2]->loop_stmt->for_loop->init);
	assert(!ast->stmts[2]->loop_stmt->for_loop->cond);
	assert(ast->stmts[2]->loop_stmt->for_loop->step);
	assert(sbuffer_len(ast->stmts[2]->loop_stmt->for_loop->body->stmts) == 1);

	assert(ast->stmts[3]->kind == STMT_LOOP);
	assert(ast->stmts[3]->loop_stmt->kind == LOOP_FOR);
	assert(!ast->stmts[3]->loop_stmt->for_loop->init);
	assert(!ast->stmts[3]->loop_stmt->for_loop->cond);
	assert(!ast->stmts[3]->loop_stmt->for_loop->step);
	assert(ast->stmts[3]->loop_stmt->for_loop->body->stmts[0]->kind == STMT_EMPTY);

	free(path_copy);
	lexer_free(lexer);
	parser_free(parser);
	clear_imported_modules();
	ast_free(ast);
}

inline void test_loop_stmts()
{
	test_do_loop_stmts();
	test_for_loop_stmts();
	test_while_loop_stmts();
}

inline void test_if_stmts()
{
	Lexer* lexer = lexer_new(
		"test\\test-cases\\parser-test-cases\\parser-ast-tests\\statement-tests\\selection_stmts\\if_else.almc", FROM_FILE);
	char* path_copy = _strdup(lexer->curr_file);
	Parser* parser = parser_new(path_copy, lex(lexer));
	AstRoot* ast = parse(parser);

	assert(sbuffer_len(ast->stmts) == 4);
	assert(ast->stmts[0]->kind == STMT_IF);
	assert(!ast->stmts[0]->if_stmt->else_body);
	assert(sbuffer_len(ast->stmts[0]->if_stmt->elifs) == 0);

	assert(ast->stmts[1]->kind == STMT_IF);
	assert(ast->stmts[1]->if_stmt->else_body);
	assert(sbuffer_len(ast->stmts[1]->if_stmt->elifs) == 0);

	assert(ast->stmts[2]->kind == STMT_IF);
	assert(!ast->stmts[2]->if_stmt->else_body);
	assert(sbuffer_len(ast->stmts[2]->if_stmt->elifs) == 2);

	assert(ast->stmts[3]->kind == STMT_IF);
	assert(ast->stmts[3]->if_stmt->else_body);
	assert(sbuffer_len(ast->stmts[3]->if_stmt->elifs) == 2);

	free(path_copy);
	lexer_free(lexer);
	parser_free(parser);
	clear_imported_modules();
	ast_free(ast);
}

inline void test_switch_stmts()
{
	Lexer* lexer = lexer_new(
		"test\\test-cases\\parser-test-cases\\parser-ast-tests\\statement-tests\\selection_stmts\\switch_case.almc", FROM_FILE);
	char* path_copy = _strdup(lexer->curr_file);
	Parser* parser = parser_new(path_copy, lex(lexer));
	AstRoot* ast = parse(parser);

	assert(sbuffer_len(ast->stmts) == 5);
	assert(ast->stmts[0]->kind == STMT_SWITCH);
	assert(!ast->stmts[0]->switch_stmt->default_case);
	assert(sbuffer_len(ast->stmts[0]->switch_stmt->cases) == 0);

	assert(ast->stmts[1]->kind == STMT_SWITCH);
	assert(ast->stmts[1]->switch_stmt->default_case);
	assert(sbuffer_len(ast->stmts[1]->switch_stmt->cases) == 0);

	assert(ast->stmts[2]->kind == STMT_SWITCH);
	assert(!ast->stmts[2]->switch_stmt->default_case);
	assert(ast->stmts[2]->switch_stmt->cases[0]->is_conjucted);
	assert(!ast->stmts[2]->switch_stmt->cases[1]->is_conjucted);
	assert(sbuffer_len(ast->stmts[2]->switch_stmt->cases) == 2);

	assert(ast->stmts[3]->kind == STMT_SWITCH);
	assert(!ast->stmts[3]->switch_stmt->default_case);
	assert(!ast->stmts[3]->switch_stmt->cases[0]->is_conjucted);
	assert(!ast->stmts[3]->switch_stmt->cases[1]->is_conjucted);
	assert(sbuffer_len(ast->stmts[3]->switch_stmt->cases) == 2);

	assert(ast->stmts[4]->kind == STMT_SWITCH);
	assert(ast->stmts[4]->switch_stmt->default_case);
	assert(!ast->stmts[4]->switch_stmt->cases[0]->is_conjucted);
	assert(!ast->stmts[4]->switch_stmt->cases[1]->is_conjucted);
	assert(sbuffer_len(ast->stmts[4]->switch_stmt->cases) == 2);

	free(path_copy);
	lexer_free(lexer);
	parser_free(parser);
	clear_imported_modules();
	ast_free(ast);
}

inline void test_selection_stmts()
{
	test_if_stmts();
	test_switch_stmts();
}

inline void test_jump_stmts()
{
	Lexer* lexer = lexer_new(
		"test\\test-cases\\parser-test-cases\\parser-ast-tests\\statement-tests\\jump_stmts.almc", FROM_FILE);
	char* path_copy = _strdup(lexer->curr_file);
	Parser* parser = parser_new(path_copy, lex(lexer));
	AstRoot* ast = parse(parser);

	assert(sbuffer_len(ast->stmts) == 5);
	for (size_t i = 0; i < sbuffer_len(ast->stmts); i++)
		assert(ast->stmts[i]->kind == STMT_JUMP);

	assert(ast->stmts[0]->jump_stmt->kind == JUMP_RETURN);
	assert(ast->stmts[0]->jump_stmt->additional_expr);

	assert(ast->stmts[1]->jump_stmt->kind == JUMP_RETURN);
	assert(!ast->stmts[1]->jump_stmt->additional_expr);

	assert(ast->stmts[2]->jump_stmt->kind == JUMP_GOTO);
	assert(ast->stmts[2]->jump_stmt->additional_expr->kind == EXPR_IDNT);

	assert(ast->stmts[3]->jump_stmt->kind == JUMP_CONTINUE);
	assert(ast->stmts[4]->jump_stmt->kind == JUMP_BREAK);

	free(path_copy);
	lexer_free(lexer);
	parser_free(parser);
	clear_imported_modules();
	ast_free(ast);
}

inline void ast_stmt_tests()
{
	test_func_decl_stmts();
	test_type_decl_stmts();
	test_var_decl_stmts();
	test_selection_stmts();
	test_jump_stmts();
	test_loop_stmts();
}

#endif