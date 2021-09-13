#include "test.h"

void print_tokens(Token* const tokens)
{
	for (int i = 0; i < sbuffer_len(tokens); i++)
		printf("%s\n", token_tostr(tokens + i));
}

void lexer_test()
{
	#define lexer_test_case_init(lexer, tokens, type, src) lexer = lexer_new(src, type), tokens = lexer_get_tokens(lexer) 

	Lexer* lexer;
	Token* tokens;
	const char* stream;

	// string lexing error test
	//lexer_test_case_init(lexer, tokens, STREAM_FILE, "C:\\Users\\HP\\source\\repos\\almc\\Debug\\test5.txt");
	//sbuffer_free(tokens);
	//----------------------------------------------------------------------

	//lexer_test_case_init(lexer, tokens, STREAM_CHAR_PTR, "18446744073709551615 0o2777777777777777777777");
	//sbuffer_free(tokens);

	//lexer_test_case_init(lexer, tokens, STREAM_CHAR_PTR, "\'cc \"asdasd d \n\' asd");
	//print_tokens(tokens);
	//sbuffer_free(tokens);

	// string lexing test in file
	lexer_test_case_init(lexer, tokens, STREAM_FILE, "C:\\Users\\HP\\source\\repos\\almc\\Debug\\test5.txt");
	assert(sbuffer_len(tokens) == 2);
	assert(strcmp(tokens[0].str_value, "hello world!!\r\n") == 0);
	assert(strcmp(tokens[1].str_value, "i like \'this\' word!!\r\n") == 0);
	print_tokens(tokens);
	sbuffer_free(tokens);

	// string lexing test in char ptr
	lexer_test_case_init(lexer, tokens, STREAM_CHAR_PTR, "\"hello world!\\n\" \"i like \\\'this\\\' word!\"");
	assert(sbuffer_len(tokens) == 2);
	assert(strcmp(tokens[0].str_value, "hello world!\n") == 0);
	assert(strcmp(tokens[1].str_value, "i like \'this\' word!") == 0);
	print_tokens(tokens);
	sbuffer_free(tokens);

	// char lexing error tests
	//lexer_test_case_init(lexer, tokens, STREAM_CHAR_PTR, "\'nn\'");
	//sbuffer_free(tokens);
	//lexer_test_case_init(lexer, tokens, STREAM_CHAR_PTR, "\'\\e\' ");
	//sbuffer_free(tokens);
	//lexer_test_case_init(lexer, tokens, STREAM_CHAR_PTR, "\'e");
	//sbuffer_free(tokens);
	//----------------------------------------------------------------------

	// char lexing test
	lexer_test_case_init(lexer, tokens, STREAM_CHAR_PTR, "\'n\' \'\\n\' \'\\f\' \'\\\\\' \'\\\'\' \'\\\"\'");
	assert(sbuffer_len(tokens) == 6);
	assert(tokens[0].char_value == 'n');
	assert(tokens[1].char_value == '\n');
	assert(tokens[2].char_value == '\f');
	assert(tokens[3].char_value == '\\');
	assert(tokens[4].char_value == '\'');
	assert(tokens[5].char_value == '\"');
	sbuffer_free(tokens);

	// number lexing test
	lexer_test_case_init(lexer, tokens, STREAM_CHAR_PTR, "0X0 0B0 0O0 0o000123 0123 0b01010 0xAbCdEf 0.123 0x00000000FFFFFFFFFFFFFFFF 0o1777777777777777777777 0b1111111111111111111111111111111111111111111111111111111111111111 ");
	assert(sbuffer_len(tokens) == 11);
	assert(tokens[0].ivalue == 0);
	assert(tokens[1].ivalue == 0);
	assert(tokens[2].ivalue == 0);
	assert(tokens[3].ivalue == 0123);
	assert(tokens[4].ivalue == 123);
	assert(tokens[5].ivalue == 0b01010);
	assert(tokens[6].ivalue == 0xabcdef);
	assert(tokens[8].ivalue == ULLONG_MAX);
	assert(tokens[9].ivalue == ULLONG_MAX);
	assert(tokens[10].ivalue == ULLONG_MAX);
	//todo: occurs bug (float point)
	//assert(tokens[3].ivalue == 0.123);
	sbuffer_free(tokens);

	// different operators (keychars) lexing test
	lexer_test_case_init(lexer, tokens, STREAM_CHAR_PTR, " ^<< >>= <<=\n ++\r -- != ) (= &&");
	assert(sbuffer_len(tokens) == 11);
	print_tokens(tokens);
	sbuffer_free(tokens);

	// file + keywords test
	lexer_test_case_init(lexer, tokens, STREAM_FILE, "C:\\Users\\HP\\source\\repos\\almc\\Debug\\test.txt");
	assert(sbuffer_len(tokens) == 14);
	assert(tokens[4].type == TOKEN_KEYWORD_DATASIZE);
	assert(tokens[5].type == TOKEN_KEYWORD_FOR);
	assert(tokens[6].type == TOKEN_KEYWORD_STRUCT);
	assert(tokens[7].type == TOKEN_KEYWORD_REGISTER);
	sbuffer_free(tokens);

	// multi comment test?
	lexer_test_case_init(lexer, tokens, STREAM_CHAR_PTR, "/*__AashdA123  ! as.*/!asda_\r\n 11 ");
	assert(sbuffer_len(tokens) == 3);
	assert(tokens[0].type == TOKEN_EXCL_MARK);
	assert(strcmp(tokens[1].str_value, "asda_") == 0);
	assert(tokens[2].ivalue == 11);
	sbuffer_free(tokens);

	// single comment test?
	lexer_test_case_init(lexer, tokens, STREAM_CHAR_PTR, "//__AashdA123  ! as.asda_ 11 ");
	assert(sbuffer_len(tokens) == 0);
	sbuffer_free(tokens);

	// first lexing test, for primitives
	lexer_test_case_init(lexer, tokens, STREAM_CHAR_PTR, "__AashdA123  %  &as.asda_ 132123  ");
	assert(sbuffer_len(tokens) == 6);
	assert(strcmp(tokens[0].str_value, "__AashdA123") == 0);
	assert(strcmp(tokens[4].str_value, "asda_") == 0);
	assert(tokens[1].type == TOKEN_MODULUS);
	assert(tokens[2].type == TOKEN_AMPERSAND);
	assert(tokens[5].ivalue == 132123);
	sbuffer_free(tokens);

	//global lexing test
	lexer_test_case_init(lexer, tokens, STREAM_FILE, "C:\\Users\\HP\\source\\repos\\almc\\Debug\\test6.txt");
	print_tokens(tokens);
	sbuffer_free(tokens);
}

void ast_print_test()
{
	Expr* idnt = expr_new(EXPR_IDNT, idnt_new("Testvar", NULL));
	Expr* lconst = expr_new(EXPR_CONST, const_new(CONST_UINT, 12323, NULL));
	Expr* rconst = expr_new(EXPR_CONST, const_new(CONST_FLOAT, 5.5555, NULL));
	Expr* unary = expr_new(EXPR_UNARY_EXPR, unary_expr_new(UNARY_BW_NOT, rconst));
	Expr* binexpr = expr_new(EXPR_BINARY_EXPR, binary_expr_new(BINARY_LG_OR, unary, idnt));
	Expr* expr = expr_new(EXPR_BINARY_EXPR, binary_expr_new(BINARY_ADD, lconst, binexpr));

	AstRoot* ast = new_s(AstRoot, ast);
	ast->exprs = NULL;
	sbuffer_add(ast->exprs, expr);
	sbuffer_add(ast->exprs, expr);

	print_ast(ast);
}

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

	int n_src = 9;
	char* str = NULL;
	char* src = "insert me";

	for (int i = 0; i < n_src; i++)
		sbuffer_add(str, src[i]);

	assert(sbuffer_cap(str) != sbuffer_len(str));
	sbuffer_rdc(str, sbuffer_len(str));
	assert(sbuffer_cap(str) == sbuffer_len(str));

	sbuffer_free(str);
}

void parser_test()
{
	char buffer[1024];
	while (1)
	{
		printf(CYAN); char* str = gets(buffer); printf(RESET);
		Lexer* l = lexer_new(str, STREAM_CHAR_PTR);
		lexer_get_tokens(l);
		Parser* p = parser_new(l);

		print_ast(parse(p));
	}
}

void run_tests()
{
	sb_test();
	parser_test();
	//ast_print_test();
	//lexer_test();
}