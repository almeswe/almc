#ifndef ALMC_GLOBAL_AST_IMPORT_TEST
#define ALMC_GLOBAL_AST_IMPORT_TEST

inline void test_case1()
{
	Lexer* lexer = lexer_new(
		"test\\test-cases\\parser-test-cases\\parser-ast-tests\\import-tests\\case_1\\a.almc", FROM_FILE);
	char* path_copy = _strdup(lexer->curr_file);
	Parser* parser = parser_new(path_copy, lex(lexer));
	AstRoot* ast = parse(parser);

	assert(ast->stmts[0]->kind == STMT_IMPORT);
	assert(ast->stmts[0]->import_stmt->ast->stmts[0]->kind == STMT_VAR_DECL);
	assert(ast->stmts[0]->import_stmt->ast->stmts[1]->kind == STMT_FUNC_DECL);
	assert(ast->stmts[0]->import_stmt->ast->stmts[2]->kind == STMT_TYPE_DECL && 
		   ast->stmts[0]->import_stmt->ast->stmts[2]->type_decl->kind == TYPE_DECL_ENUM);
	assert(ast->stmts[0]->import_stmt->ast->stmts[3]->kind == STMT_TYPE_DECL && 
		   ast->stmts[0]->import_stmt->ast->stmts[3]->type_decl->kind == TYPE_DECL_STRUCT);
	assert(ast->stmts[0]->import_stmt->ast->stmts[4]->kind == STMT_TYPE_DECL && 
		   ast->stmts[0]->import_stmt->ast->stmts[4]->type_decl->kind == TYPE_DECL_UNION);
	free(path_copy);
	lexer_free(lexer);
	parser_free(parser);
	clear_imported_modules();
	ast_free(ast);
}
inline void test_case2()
{
	Lexer* lexer = lexer_new(
		"test\\test-cases\\parser-test-cases\\parser-ast-tests\\import-tests\\case_2\\a.almc", FROM_FILE);
	char* path_copy = _strdup(lexer->curr_file);
	Parser* parser = parser_new(path_copy, lex(lexer));
	AstRoot* ast = parse(parser);

	assert(ast->stmts[0]->kind == STMT_IMPORT);
	assert(ast->stmts[0]->import_stmt->ast->stmts[0]->kind == STMT_VAR_DECL);
	assert(ast->stmts[0]->import_stmt->ast->stmts[1]->kind == STMT_TYPE_DECL &&
		   ast->stmts[0]->import_stmt->ast->stmts[1]->type_decl->kind == TYPE_DECL_UNION);

	free(path_copy);
	lexer_free(lexer);
	parser_free(parser);
	clear_imported_modules();
	ast_free(ast);
}
inline void test_case3()
{
	Lexer* lexer = lexer_new(
		"test\\test-cases\\parser-test-cases\\parser-ast-tests\\import-tests\\case_3\\a.almc", FROM_FILE);
	char* path_copy = _strdup(lexer->curr_file);
	Parser* parser = parser_new(path_copy, lex(lexer));
	AstRoot* ast = parse(parser);

	// first from-stmt
	assert(ast->stmts[0]->kind == STMT_IMPORT);
	assert(ast->stmts[0]->import_stmt->ast->stmts[0]->kind == STMT_VAR_DECL);
	
	// second from-stmt
	assert(ast->stmts[1]->kind == STMT_IMPORT);
	assert(ast->stmts[1]->import_stmt->ast->stmts[0]->kind == STMT_TYPE_DECL &&
		ast->stmts[1]->import_stmt->ast->stmts[0]->type_decl->kind == TYPE_DECL_STRUCT);

	// import stmt
	assert(ast->stmts[2]->kind == STMT_IMPORT);
	assert(ast->stmts[2]->import_stmt->ast->stmts[0]->kind == STMT_FUNC_DECL);
	assert(ast->stmts[2]->import_stmt->ast->stmts[1]->kind == STMT_TYPE_DECL &&
		ast->stmts[2]->import_stmt->ast->stmts[1]->type_decl->kind == TYPE_DECL_ENUM);
	assert(ast->stmts[2]->import_stmt->ast->stmts[2]->kind == STMT_TYPE_DECL &&
		ast->stmts[2]->import_stmt->ast->stmts[2]->type_decl->kind == TYPE_DECL_UNION);

	free(path_copy);
	lexer_free(lexer);
	parser_free(parser);
	clear_imported_modules();
	ast_free(ast);
}
inline void test_case4()
{
	Lexer* lexer = lexer_new(
		"test\\test-cases\\parser-test-cases\\parser-ast-tests\\import-tests\\case_4\\a.almc", FROM_FILE);
	char* path_copy = _strdup(lexer->curr_file);
	Parser* parser = parser_new(path_copy, lex(lexer));
	AstRoot* ast = parse(parser);

	// import stmt
	assert(ast->stmts[0]->kind == STMT_IMPORT);
	assert(ast->stmts[0]->import_stmt->ast->stmts[0]->kind == STMT_VAR_DECL);
	assert(ast->stmts[0]->import_stmt->ast->stmts[1]->kind == STMT_FUNC_DECL);
	assert(ast->stmts[0]->import_stmt->ast->stmts[2]->kind == STMT_TYPE_DECL &&
		ast->stmts[0]->import_stmt->ast->stmts[2]->type_decl->kind == TYPE_DECL_ENUM);
	assert(ast->stmts[0]->import_stmt->ast->stmts[3]->kind == STMT_TYPE_DECL &&
		ast->stmts[0]->import_stmt->ast->stmts[3]->type_decl->kind == TYPE_DECL_STRUCT);
	assert(ast->stmts[0]->import_stmt->ast->stmts[4]->kind == STMT_TYPE_DECL &&
		ast->stmts[0]->import_stmt->ast->stmts[4]->type_decl->kind == TYPE_DECL_UNION);
	
	// from-stmt
	assert(ast->stmts[1]->kind == STMT_IMPORT);
	assert(!ast->stmts[1]->import_stmt->ast->stmts);

	// from-stmt
	assert(ast->stmts[2]->kind == STMT_IMPORT);
	assert(!ast->stmts[2]->import_stmt->ast->stmts);

	free(path_copy);
	lexer_free(lexer);
	parser_free(parser);
	clear_imported_modules();
	ast_free(ast);
}
inline void test_case5()
{
	Lexer* lexer = lexer_new(
		"test\\test-cases\\parser-test-cases\\parser-ast-tests\\import-tests\\case_5\\a.almc", FROM_FILE);
	char* path_copy = _strdup(lexer->curr_file);
	Parser* parser = parser_new(path_copy, lex(lexer));
	AstRoot* ast = parse(parser);

	// import stmt
	assert(ast->stmts[0]->kind == STMT_IMPORT);
	assert(ast->stmts[0]->import_stmt->ast->stmts[0]->kind == STMT_FUNC_DECL);
	assert(ast->stmts[0]->import_stmt->ast->stmts[1]->kind == STMT_FUNC_DECL);

	// import-stmt
	assert(ast->stmts[1]->kind == STMT_IMPORT);
	assert(!ast->stmts[1]->import_stmt->ast->stmts);

	// from-stmt
	assert(ast->stmts[2]->kind == STMT_IMPORT);
	assert(ast->stmts[2]->import_stmt->ast->stmts[0]->kind == STMT_VAR_DECL);

	// from-stmt
	assert(ast->stmts[3]->kind == STMT_IMPORT);
	assert(!ast->stmts[3]->import_stmt->ast->stmts);

	free(path_copy);
	lexer_free(lexer);
	parser_free(parser);
	clear_imported_modules();
	ast_free(ast);
}
inline void test_case6()
{
	Lexer* lexer = lexer_new(
		"test\\test-cases\\parser-test-cases\\parser-ast-tests\\import-tests\\case_6\\a.almc", FROM_FILE);
	char* path_copy = _strdup(lexer->curr_file);
	Parser* parser = parser_new(path_copy, lex(lexer));
	AstRoot* ast = parse(parser);

	// import stmt
	assert(ast->stmts[0]->kind == STMT_IMPORT);
	assert(ast->stmts[0]->import_stmt->ast->stmts[0]->kind == STMT_VAR_DECL);
	assert(ast->stmts[0]->import_stmt->ast->stmts[1]->kind == STMT_FUNC_DECL);
	assert(ast->stmts[0]->import_stmt->ast->stmts[2]->kind == STMT_TYPE_DECL &&
		ast->stmts[0]->import_stmt->ast->stmts[2]->type_decl->kind == TYPE_DECL_ENUM);
	assert(ast->stmts[0]->import_stmt->ast->stmts[3]->kind == STMT_TYPE_DECL &&
		ast->stmts[0]->import_stmt->ast->stmts[3]->type_decl->kind == TYPE_DECL_STRUCT);
	assert(ast->stmts[0]->import_stmt->ast->stmts[4]->kind == STMT_TYPE_DECL &&
		ast->stmts[0]->import_stmt->ast->stmts[4]->type_decl->kind == TYPE_DECL_UNION);

	// import-stmt
	assert(ast->stmts[1]->kind == STMT_IMPORT);	
	assert(ast->stmts[1]->import_stmt->ast->stmts[0]->kind == STMT_IMPORT);
	assert(!ast->stmts[1]->import_stmt->ast->stmts[0]->import_stmt->ast->stmts);
	assert(ast->stmts[1]->import_stmt->ast->stmts[1]->kind == STMT_FUNC_DECL);

	free(path_copy);
	lexer_free(lexer);
	parser_free(parser);
	clear_imported_modules();
	ast_free(ast);
}
inline void test_case7()
{
	Lexer* lexer = lexer_new(
		"test\\test-cases\\parser-test-cases\\parser-ast-tests\\import-tests\\case_7\\a.almc", FROM_FILE);
	char* path_copy = _strdup(lexer->curr_file);
	Parser* parser = parser_new(path_copy, lex(lexer));
	AstRoot* ast = parse(parser);

	// import stmt
	assert(ast->stmts[0]->kind == STMT_IMPORT);
	assert(ast->stmts[0]->import_stmt->ast->stmts[0]->kind == STMT_FUNC_DECL);

	free(path_copy);
	lexer_free(lexer);
	parser_free(parser);
	clear_imported_modules();
	ast_free(ast);
}

inline void test_import_all_cases()
{
	test_case1();
	test_case2();
	test_case3();
	test_case4();
	test_case5();
	test_case6();
	test_case7();
}

#endif