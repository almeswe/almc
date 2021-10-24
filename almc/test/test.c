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
}*/

void run_tests()
{
	lexer_run_tests();
	test_import_all_cases();
	ast_expr_eval_run_tests();
	ast_stmt_tests();
	ast_manual_test();
}