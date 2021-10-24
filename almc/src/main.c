#include "..\test\test.h"

#include "back-end/x86 asm/gen.h"

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
		init_reserved_registers();
		gen_expr(expr);
	}
}

int main(int argc, char** argv)
{
	//test();
	run_tests();
	return 0;
}