#include "test.h"

/*void sb_test()
{
	int n = 1024;
	int* buf = NULL;

	for (int i = 0; i < n; i++)
	{
		sbuffer_add(buf, i);
		assert(buf[i] == i);
	}
	assert(n == sbuffer_len(buf));

	sbuffer_pop(buf);
	sbuffer_pop(buf);
	assert(n - 2 == sbuffer_len(buf));

	sbuffer_free(buf);

	int n_src = 9;
	char* str = NULL;
	char* src = "insert me";

	for (int i = 0; i < n_src; i++)
		sbuffer_add(str, src[i]);

	assert(sbuffer_cap(str) != sbuffer_len(str));
	sbuffer_rdc(str, sbuffer_len(str));
	assert(sbuffer_cap(str) == sbuffer_len(str));

	sbuffer_free(str);
}*/

void parser_expr_manual_test()
{
	char buffer[1024];
	while (1)
	{
			
		char* str = gets(buffer);
		Lexer* l = lexer_new(str, FROM_CHAR_PTR);
		Parser* p = parser_new(l->curr_file, lex(l));
		Expr* root = parse_expr(p);
		printf("result: %d\n", eval_expr(root));
		print_expr(root, "");
	}
}

void parser_stmt_manual_test()
{
	const char* file = "C:\\Users\\almeswe\\source\\repos\\almc\\Debug\\stmt-test.txt";
	while (1)
	{ 
		Lexer* l = lexer_new(file, FROM_FILE);
		Parser* p = parser_new(l->curr_file, lex(l));
		AstRoot* root = parse(p);
		print_ast(root, "");
		ast_free(root);
		lexer_free(l);
		parser_free(p);
		int a = getchar();
	}
}

void run_tests()
{
	lexer_run_tests();
	ast_expr_eval_run_tests();
	//parser_expr_manual_test();
	parser_stmt_manual_test();
}