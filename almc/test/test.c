#include "test.h"
#include "..\src\front-end\ast-print.h"
#include "..\src\front-end\ast-expr-eval.h"

void print_tokens(Token* const tokens)
{
	for (uint32_t i = 0; i < sbuffer_len(tokens); i++)
		printf("%s\n", token_tostr(tokens + i));
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
void lexer_test()
{
	#define lexer_test_case_init(lexer, tokens, type, src) \
		lexer = lexer_new(src, type), tokens = lex(lexer) 
	#define lexer_test_case_free(lexer) \
		lexer_free(lexer);

	Lexer* lexer = 0;
	Token* tokens = 0;
	const char* stream = 0;

	//lexing table test
	lexer_test_case_init(lexer, tokens, STREAM_FILE, "C:\\Users\\HP\\source\\repos\\almc\\Debug\\test5.txt");

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
	assert(sbuffer_len(tokens) == 3);
	assert(strcmp(tokens[0].str_value, "hello world!!\r\n") == 0);
	assert(strcmp(tokens[1].str_value, "i like \'this\' word!!\r\n") == 0);
	//print_tokens(tokens);
	lexer_test_case_free(lexer);

	// string lexing test in char ptr
	lexer_test_case_init(lexer, tokens, STREAM_CHAR_PTR, "\"hello world!\\n\" \"i like \\\'this\\\' word!\"");
	assert(sbuffer_len(tokens) == 3);
	assert(strcmp(tokens[0].str_value, "hello world!\n") == 0);
	assert(strcmp(tokens[1].str_value, "i like \'this\' word!") == 0);
	//print_tokens(tokens);
	lexer_test_case_free(lexer);

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
	assert(sbuffer_len(tokens) == 7);
	assert(tokens[0].char_value == 'n');
	assert(tokens[1].char_value == '\n');
	assert(tokens[2].char_value == '\f');
	assert(tokens[3].char_value == '\\');
	assert(tokens[4].char_value == '\'');
	assert(tokens[5].char_value == '\"');
	lexer_test_case_free(lexer);

	// number lexing test
	lexer_test_case_init(lexer, tokens, STREAM_CHAR_PTR, "0X0 0B0 0O0 0o000123 0123 0b01010 0xAbCdEf 0.123 0x00000000FFFFFFFFFFFFFFFF 0o1777777777777777777777 0b1111111111111111111111111111111111111111111111111111111111111111 ");
	assert(sbuffer_len(tokens) == 12);
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
	lexer_test_case_free(lexer);

	// different operators (keychars) lexing test
	lexer_test_case_init(lexer, tokens, STREAM_CHAR_PTR, " ^<< >>= <<=\n ++\r -- != ) (= &&");
	assert(sbuffer_len(tokens) == 12);
	//print_tokens(tokens);
	sbuffer_free(tokens);

	// file + keywords test
	lexer_test_case_init(lexer, tokens, STREAM_FILE, "C:\\Users\\HP\\source\\repos\\almc\\Debug\\test.txt");
	assert(sbuffer_len(tokens) == 11);
	assert(tokens[0].type == TOKEN_KEYWORD_UINT8);
	assert(tokens[1].type == TOKEN_KEYWORD_FOR);
	assert(tokens[2].type == TOKEN_KEYWORD_STRUCT);
	assert(tokens[3].type == TOKEN_KEYWORD_REGISTER);
	lexer_test_case_free(lexer);

	// first lexing test, for primitives
	lexer_test_case_init(lexer, tokens, STREAM_CHAR_PTR, "__AashdA123  %  &as.asda_ 132123  ");
	assert(sbuffer_len(tokens) == 8);
	assert(strcmp(tokens[0].str_value, "__AashdA123") == 0);
	assert(strcmp(tokens[5].str_value, "asda_") == 0);
	assert(tokens[1].type == TOKEN_MODULUS);
	assert(tokens[2].type == TOKEN_AMPERSAND);
	assert(tokens[6].ivalue == 132123);
	lexer_test_case_free(lexer);

	//global lexing test
	lexer_test_case_init(lexer, tokens, STREAM_FILE, "C:\\Users\\HP\\source\\repos\\almc\\Debug\\test6.txt");
	//print_tokens(tokens);
	lexer_test_case_free(lexer);
}

void parser_expr_manual_test()
{
	char buffer[1024];
	while (1)
	{
		char* str = gets(buffer);
		Lexer* l = lexer_new(str, STREAM_CHAR_PTR);
		Parser* p = parser_new(lex(l));
		Expr* root = parse_expr(p);
		print_expr(root, "");
	}
}

void parser_stmt_manual_test()
{
	const char* file = "C:\\Users\\almeswe\\source\\repos\\almc\\Debug\\stmt-test.txt";
	while (1)
	{ 
		Lexer* l = lexer_new(file, STREAM_FILE);
		Parser* p = parser_new(lex(l));
		Stmt* root = parse_stmt(p);
		print_stmt(root, "");
		stmt_free(root);
		int a = getchar();
	}
}

void parser_eval_test()
{
	/*
		Tests for checking the ast creation for simple expressions via evaluation
	*/
#define parser_eval_test_case_free(lexer, parser)    \
	lexer_free(lexer);  \
	free(parser);       \

#define parser_eval_test_case_init(expr, str_expr) \
	l = lexer_new(str_expr, STREAM_CHAR_PTR);      \
	p = parser_new(lex(l));						   \
	ast = parse_expr(p);						   \
	assert((expr) == eval_expr(ast)); 			   \
	parser_eval_test_case_free(l, p)

	Lexer* l;
	Parser* p;
	Expr* ast;

	parser_eval_test_case_init(+-+-1, "+-+-1");
	parser_eval_test_case_init(- - - 1, "- - - 1");
	parser_eval_test_case_init(+-+-0xff, "+-+-0xff");
	parser_eval_test_case_init(!0, "!0");
	parser_eval_test_case_init(~0b011, "~0b011");
	
	parser_eval_test_case_init(0123 - 22, "0o123 - 22");
	parser_eval_test_case_init(2 + 2, "2+2");
	
	parser_eval_test_case_init(5 * 6, "5 * 6");
	parser_eval_test_case_init(6 % 2, "6 % 2");
	parser_eval_test_case_init(10 / 2, "10 / 2");
	
	parser_eval_test_case_init(0b0011 << 2, "0b0011 << 2");
	parser_eval_test_case_init(0b1100 >> 2, "0b1100 >> 2");
	
	parser_eval_test_case_init(2 < 3, "2 < 3");
	parser_eval_test_case_init(123 > 0123, "123 > 0o123");
	parser_eval_test_case_init(10 <= 2, "10 <= 2");
	parser_eval_test_case_init(10 >= 2, "10 >= 2");

	parser_eval_test_case_init(15 == 15, "15 == 15");
	parser_eval_test_case_init(15 == 14, "15 == 14");
	parser_eval_test_case_init(22 != 22, "22 != 22");	
	parser_eval_test_case_init(23 != 22, "23 != 22");

	parser_eval_test_case_init(0b0011 & 0b1100, "0b0011 & 0b1100");
	parser_eval_test_case_init(0b0111 ^ 0b1110, "0b0111 ^ 0b1110");
	parser_eval_test_case_init(0b0011 | 0b1100, "0b0011 | 0b1100");

	parser_eval_test_case_init(1 && 1, "1 && 1");
	parser_eval_test_case_init(1 && 0, "1 && 0");
	parser_eval_test_case_init(0 && 1, "0 && 1");
	parser_eval_test_case_init(0 && 0, "0 && 0");

	parser_eval_test_case_init(1 || 1, "1 || 1");
	parser_eval_test_case_init(1 || 0, "1 || 0");
	parser_eval_test_case_init(0 || 1, "0 || 1");
	parser_eval_test_case_init(0 || 0, "0 || 0");

	parser_eval_test_case_init(1 ? 1 : 0, "1 ? 1 : 0");
	parser_eval_test_case_init(0 ? 1 : 0, "0 ? 1 : 0");

	parser_eval_test_case_init(((-+-+-+-+-+0xff + 123) - ((0b011 * 44 % 3) << 4)) && 1 == 0,
		"((-+-+-+-+-+0xff + 123) - ((0b011 * 44 % 3) << 4)) && 1 == 0");
	parser_eval_test_case_init(((-+-+-+-+-+0xff + 123) - ((0b011 * 44 % 3) << 4)) && 1 == 0 ? 213 >> 5 % 4 ^ 123 == 5 : (123 & 23 ^ (123 || 00123 != 33 * 4)),
		"((-+-+-+-+-+0xff + 123) - ((0b011 * 44 % 3) << 4)) && 1 == 0 ? 213 >> 5 % 4 ^ 123 == 5 : (123 & 23 ^ (123 || 0o123 != 33 * 4))");
	parser_eval_test_case_init(213 >> 5 % 4 ^ 123 == 5 ? (2123 * 2 <= 5 <= 123 * 5 >> 5 << 5 ? ((0x123 * 00123 > 1000 == 1 << 10)) : (0b010101010 * 4 | (33 << 2 * 4))) : (-23 - 32 - 123 * 5 | 123 && 9 * 3),
		"213 >> 5 % 4 ^ 123 == 5 ? (2123 * 2 <= 5 <= 123 * 5 >> 5 << 5 ? ((0x123 * 0o123 > 1000 == 1 << 10)) : (0b010101010 * 4 | (33 << 2 * 4))) : (- 23-32- 123 * 5 | 123 && 9 * 3)");
}

void run_tests()
{
	//sb_test();
	//lexer_test();
	parser_eval_test();
	//parser_expr_manual_test();
	parser_stmt_manual_test();
}