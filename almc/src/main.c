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
		
		for (int i = 0; i < sbuffer_len(ast->stmts); i++)
			gen_stmt(ast->stmts[i], NULL);
		//print_expr(expr, "");
		//init_reserved_registers();
		char c = getchar();
		system("cls");
	}
}

int type_test()
{
	while (1)
	{
		char buffer[50];
		char* input = gets(buffer);
		Lexer* lexer = lexer_new(
			input, FROM_CHAR_PTR);
		Parser* parser = parser_new(NULL, lex(lexer));
		Expr* expr = parse_expr(parser);

		print_type(get_expr_type(expr), "");

		lexer_free(lexer);
		parser_free(parser);
		expr_free(expr);
		clear_imported_modules();
	}
}

int cast_test()
{
	char* types[] = {
		"u8",
		"i8",
		"chr",
		"u16",
		"i16",
		"u32",
		"i32",
		"u64",
		"i64",
		"f32",
		"f64"
	};
	Type* type = cnew_s(Type, type, 1);
	Type* type1 = cnew_s(Type, type1, 1);

	for (int i = 0; i < 11; i++)
		for (int j = 0; j < 11; j++)
		{
			type->repr = types[i];
			type1->repr = types[j];
			printf("Cast %s to %s is %d\n", type1->repr, type->repr, can_cast_implicitly(type, type1));
		}
}

int main(int argc, char** argv)
{
	//test();
	//cast_test();
	type_test();
	run_tests();
	return 0;
}