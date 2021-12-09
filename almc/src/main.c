#include "..\test\test.h"
#include "..\src\front-end\types.h"
#include "back-end/x86 asm/stack-frame.h"
#include "back-end/x86 asm/gen.h"

int back_end_test()
{
		/*Lexer* lexer = lexer_new(
			"test\\test-cases\\parser-test-cases\\parser-ast-manual-tests\\testfolder\\back_end_test.almc", FROM_FILE);
		char* path_copy = _strdup(lexer->curr_file);
		Parser* parser = parser_new(path_copy, lex(lexer));
		AstRoot* ast = parse(parser);

		Visitor* visitor = visitor_new();
		visit(ast, visitor);

		for (int i = 0; i < sbuffer_len(ast->stmts); i++)
			gen_stmt(ast->stmts[i], NULL);
		//print_expr(expr, "");
		//init_reserved_registers();
		char c = getchar();
		system("cls");
		*/
		Lexer* lexer = lexer_new(
			"fnc entry main() : void {}", FROM_CHAR_PTR);
		Parser* parser = parser_new(NULL, lex(lexer));
		AstRoot* ast = parse(parser);
		Visitor* visitor = visitor_new();
		visit(ast, visitor);

		AsmProgram* program = gen(ast);
		print_program(program);
		//program_free(program);
}

int main(int argc, char** argv)
{
	back_end_test();
	//test();
	//type_test();
	//type2_test();
	//run_tests();
	return 0;
}