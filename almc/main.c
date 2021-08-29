#include <assert.h>
#include "lexer.h"
#include "sbuffer.h"

void lex_test()
{
	Lexer* l = lexer_new("13123   _asdas Aas123 123", STREAM_CHAR_PTR);
	Token* tkns = lexer_get_tokens(l);
	//lexer_start("SJDFHS_12?312asd!kj2.!");
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