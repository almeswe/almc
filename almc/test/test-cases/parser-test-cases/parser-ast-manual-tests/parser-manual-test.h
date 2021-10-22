#ifndef ALMC_GLOBAL_MANUAL_AST_TEST
#define ALMC_GLOBAL_MANUAL_AST_TEST

#include "..\..\..\..\src\front-end\front-end.h"

inline void ast_manual_test()
{
	const char* file = 
		"test\\test-cases\\parser-test-cases\\parser-ast-manual-tests\\testfile.almc";
	while (1)
	{
		Lexer* l = lexer_new(file, FROM_FILE);
		Parser* p = parser_new(_strdup(l->curr_file), lex(l));
		AstRoot* root = parse(p);
		print_ast(root);
		lexer_free(l);
		parser_free(p);
		ast_free(root);
		clear_imported_modules();
		int a = getchar();
	}
}

#endif