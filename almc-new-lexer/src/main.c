//#include "../test/test.h"
#include "..\src\front-end\new-lexer\lexer.h"

void main(int argc, char** argv)
{
	//Lexer* l = lexer_new("123 1e+4 1.123 1.11e-5", FROM_CHAR_PTR);
	//"C:\\Users\\almeswe\\source\\repos\\almc\\Debug\\new-lexer-test.txt"
	//Lexer* l = lexer_new("C:\\Users\\almeswe\\source\\repos\\almc\\Debug\\new-lexer-test.txt", FROM_FILE);
	Lexer* l = lexer_new("1. 0011 0xfff 0XFF 0o123 0O33 0b101010 0B1111 133.112   123.4e+5\n\r 63\n\r1e+15", FROM_CHAR_PTR);
	Token** tokens = lex(l);
	for (int i = 0; i < sbuffer_len(tokens); i++)
		printf("%s\n", token_tostr(tokens[i]));
	//run_tests();
	system("pause");
}