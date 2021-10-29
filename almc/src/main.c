#include "..\test\test.h"

#include "back-end/x86 asm/stack-frame.h"
#include "back-end/x86 asm/gen.h"

int test()
{
	while (1)
	{
		Lexer* lexer = lexer_new(
			"test\\test-cases\\parser-test-cases\\parser-ast-manual-tests\\testfolder\\back_end_test.almc", FROM_FILE);
		char* path_copy = _strdup(lexer->curr_file);
		Parser* parser = parser_new(path_copy, lex(lexer));
		AstRoot* ast = parse(parser);
		
		StackFrame* frame = stack_frame_new(ast->stmts[0]->func_decl);
		//for (int i = 0; i < sbuffer_len(ast->stmts); i++)
		gen_func_decl(ast->stmts[0]->func_decl, frame);
		//print_expr(expr, "");
		//init_reserved_registers();
		char c = getchar();
		system("cls");
	}
}

int main(int argc, char** argv)
{
	test();
	run_tests();
	return 0;
}