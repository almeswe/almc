#define _CRT_SECURE_NO_WARNINGS

#include <assert.h>
#include "lexer.h"
#include "sbuffer.h"

#define lexer_test_case_init(lexer, tokens, type, src) lexer = lexer_new(src, type), tokens = lexer_get_tokens(lexer) 

void print_tokens(Token* const tokens)
{
	for (int i = 0; i < sbuffer_len(tokens); i++)
		printf("Token: %d, type: %d (line: %d, pos: %d, by %d)\n",
			i+1, tokens[i].type, tokens[i].context->line, tokens[i].context->start, tokens[i].context->size);
}

void lexer_test()
{
	Lexer* lexer;
	Token* tokens;
	const char* stream;

	// number lexing test
	lexer_test_case_init(lexer, tokens, STREAM_CHAR_PTR, "123 0b01010 0xAbCdEf 0.123 0xFFFFFFFFFFFFFFFF 0b1111111111111111111111111111111111111111111111111111111111111111 0x0 0b0");
	assert(sbuffer_len(tokens) == 8);
	assert(tokens[0].ivalue == 123);
	assert(tokens[1].ivalue == 0b01010);
	assert(tokens[2].ivalue == 0xabcdef);
	assert(tokens[4].ivalue == ULLONG_MAX);
	assert(tokens[5].ivalue == ULLONG_MAX);
	assert(tokens[6].ivalue == 0);
	assert(tokens[7].ivalue == 0);
	//todo: occurs bug
	//assert(tokens[3].ivalue == 0.123);
	sbuffer_free(tokens);

	// different operators (keychars) lexing test
	lexer_test_case_init(lexer, tokens, STREAM_CHAR_PTR, " ^<< <<=\n ++\r -- != ) (= &&");
	assert(sbuffer_len(tokens) == 10);
	print_tokens(tokens);
	sbuffer_free(tokens);

	// file + keywords test
	lexer_test_case_init(lexer, tokens, STREAM_FILE, "C:\\Users\\HP\\source\\repos\\almc\\Debug\\test.txt");
	assert(sbuffer_len(tokens) == 14);
	assert(tokens[4].type == TOKEN_KEYWORD_SIZEOF);
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

void test()
{
	sb_test();
	lexer_test();
}

int main(int argc, char** argv)
{
	test();
	system("pause");
	return 0;
}