#include "..\test\test.h"
#include "..\src\front-end\type.h"
#include "back-end/x86/program.h"

int back_end_test()
{
	while (1)
	{
		Lexer* lexer = lexer_new("test\\test-cases\\parser-test-cases\\parser-ast-manual-tests\\testfolder\\back_end_test.almc",
			FROM_FILE);
		Parser* parser = parser_new(lexer->curr_file, lex(lexer));
		AstRoot* ast = parse(parser);
		Visitor* visitor = visitor_new();
		visit(ast, visitor);

		AsmProgram* program = gen(ast, visitor->global);
		print_program(program);
		program_free(program);
		char c = getchar();
		system("cls");
	}
}

void type2_test()
{
	while (1)
	{
		char buffer[30];
		char* input = gets(buffer);
		system("cls");

		Lexer* lexer = lexer_new(
			input, FROM_CHAR_PTR);
		Parser* parser = parser_new(NULL, lex(lexer));
		Type* type;
		print_type(type = parse_type(parser), "");
		type_free(type);
	}
}

void cast_type_test()
{
	Lexer* lexer = lexer_new(
		"sizeof(i32)", FROM_CHAR_PTR);
	Parser* parser = parser_new(NULL, lex(lexer));
	Expr* expr = parse_expr(parser);
	return;
}

int main(int argc, char** argv)
{
	back_end_test();
	
	//printf("%s%-*iX\n", text, 50 - (int)strlen(text), num);
	run_tests();
	return 0;
}