#include <assert.h>
#include "lexer.h"
#include "sbuffer.h"

void lex_test()
{
	//TODO: FIX FILE 
	//todo: add tests for keywords and etc..
	/*const char* path = "C:\\Users\\HP\\source\\repos\\almc\\Debug\\test.txt";
	Lexer* l = lexer_new(path, STREAM_FILE);
	Token* tkns = lexer_get_tokens(l);*/

	const char* ch_stream = "//__AashdA123  ! as.asda_\n\r 132123  ";
	Lexer* lexer = lexer_new(ch_stream, STREAM_CHAR_PTR);
	Token* tokens = lexer_get_tokens(lexer);
	assert(sbuffer_len(tokens) == 1);
	assert(tokens[0].type == TOKEN_NUM && tokens[0].int_value == 132123);

	ch_stream = "__AashdA123  ! as.asda_ 132123  ";
	lexer = lexer_new(ch_stream, STREAM_CHAR_PTR);
	tokens = lexer_get_tokens(lexer);
	assert(sbuffer_len(tokens) == 4);
	assert(tokens[0].type == TOKEN_IDNT && strcmp(tokens[0].idnt_value, "__AashdA123") == 0);
	assert(tokens[2].type == TOKEN_IDNT && strcmp(tokens[2].idnt_value, "asda_") == 0);
	assert(tokens[3].type == TOKEN_NUM && tokens[3].int_value == 132123);
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
	lex_test();
}

int main(int argc, char** argv)
{
	test();
	return 0;
}