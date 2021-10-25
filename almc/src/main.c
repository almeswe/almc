#include "..\test\test.h"

#include "back-end/x86 asm/expr-gen.h"

int test()
{
	while (1)
	{
		char buffer[60] = { 0 };
		char* s = gets(buffer);
		Lexer* lexer = lexer_new(
			buffer, FROM_CHAR_PTR);
		char* path_copy = _strdup(lexer->curr_file);
		Parser* parser = parser_new(path_copy, lex(lexer));
		Expr* expr = parse_expr(parser);
		//print_expr(expr, "");
		//init_reserved_registers();
		ExprGenerator* expr_gen = expr_gen_new();
		gen_expr(expr_gen, expr);
		expr_gen_free(expr_gen);
	}
}

int main(int argc, char** argv)
{
	test();
	run_tests();
	return 0;
}