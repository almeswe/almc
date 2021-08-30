#include <assert.h>
#include "lexer.h"
#include "sbuffer.h"

#define lexer_test_case_init(lexer, tokens, type, src) lexer = lexer_new(src, type), tokens = lexer_get_tokens(lexer) 

void lex_test()
{
	Lexer* lexer;
	Token* tokens;
	const char* stream;
	
	lexer_test_case_init(lexer, tokens, STREAM_FILE, "C:\\Users\\HP\\source\\repos\\almc\\Debug\\test.txt");
	assert(sbuffer_len(tokens) == 10);
	assert(tokens[2].type == TOKEN_SIZEOF);
	assert(tokens[3].type == TOKEN_FOR);
	assert(tokens[4].type == TOKEN_STRUCT);
	assert(tokens[5].type == TOKEN_REGISTER);
	sbuffer_free(tokens);

	lexer_test_case_init(lexer, tokens, STREAM_FILE, "C:\\Users\\HP\\source\\repos\\almc\\Debug\\test1.txt");
	assert(sbuffer_len(tokens) == 6);
	sbuffer_free(tokens);

	lexer_test_case_init(lexer, tokens, STREAM_CHAR_PTR, "//__AashdA123  ! as.asda_\r\n 11 ");
	assert(sbuffer_len(tokens) == 1);
	assert(tokens[0].int_value == 11);
	sbuffer_free(tokens);

	lexer_test_case_init(lexer, tokens, STREAM_CHAR_PTR, "//__AashdA123  ! as.asda_ 11 ");
	assert(sbuffer_len(tokens) == 0);
	sbuffer_free(tokens);

	lexer_test_case_init(lexer, tokens, STREAM_CHAR_PTR, "__AashdA123  ! as.asda_ 132123  ");
	assert(sbuffer_len(tokens) == 4);
	assert(strcmp(tokens[0].idnt_value, "__AashdA123") == 0);
	assert(strcmp(tokens[2].idnt_value, "asda_") == 0);
	assert(tokens[3].int_value == 132123);
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

void separate_test()
{
	FILE* file;
	fopen_s(&file, "C:\\Users\\HP\\source\\repos\\almc\\Debug\\test.txt", "rb");
	assert(file);
	
	//todo: somekind of undo here
	char c = fseek(file, -1, SEEK_CUR);
	c = fgetc(file);
}

void test()
{
	//separate_test();
	sb_test();
	lex_test();
}

int main(int argc, char** argv)
{
	test();
	return 0;
}