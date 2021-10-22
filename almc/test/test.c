#include "test.h"

void sb_test()
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
}

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

void run_tests()
{
	sb_test();
	lexer_run_tests();
	test_import_all_cases();
	ast_expr_eval_run_tests();
	//parser_expr_manual_test();
	ast_manual_test();
}