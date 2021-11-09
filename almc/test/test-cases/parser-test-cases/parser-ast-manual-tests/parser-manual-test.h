#ifndef ALMC_GLOBAL_MANUAL_AST_TEST
#define ALMC_GLOBAL_MANUAL_AST_TEST

inline void ast_manual_test()
{
	const char* file = 
		"test\\test-cases\\parser-test-cases\\parser-ast-manual-tests\\testfolder\\testfile.almc";
	while (1)
	{
		system("cls");
		Lexer* l = lexer_new(file, FROM_FILE);
		Parser* p = parser_new(_strdup(l->curr_file), lex(l));
		AstRoot* root = parse(p);
		print_ast(root);
		lexer_free(l);
		ast_free(root);
		parser_free(p);
		clear_imported_modules();
		int a = getchar();
	}
}

#endif