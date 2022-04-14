#ifndef _ALMC_GLOBAL_MANUAL_AST_TEST
#define _ALMC_GLOBAL_MANUAL_AST_TEST

inline void manual_test()
{
	const char* file = 
		"test\\test-cases\\parser-test-cases\\parser-ast-manual-tests\\testfolder\\testfile.almc";
	while (1)
	{
		system("cls");
		Lexer* lexer = lexer_new(file, FROM_FILE);
		Parser* parser = parser_new(_strdup(lexer->curr_file), lex(lexer));
		AstRoot* ast = parse(parser);
		Visitor* visitor = visitor_new();

		visit(ast, visitor);
		print_ast(ast);
		lexer_free(lexer);
		ast_free(ast);
		parser_free(parser);
		clear_imported_modules();
		visitor_free(visitor);
		int a = getchar();
	}
}

#endif // _ALMC_GLOBAL_MANUAL_AST_TEST