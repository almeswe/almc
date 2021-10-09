//#include "../test/test.h"
#include "..\src\front-end\new-lexer\lexer.h"

void main(int argc, char** argv)
{
	//Lexer* l = lexer_new("123 1e+4 1.123 1.11e-5", FROM_CHAR_PTR);
	//"C:\\Users\\almeswe\\source\\repos\\almc\\Debug\\new-lexer-test.txt"
	//Lexer* l = lexer_new("C:\\Users\\almeswe\\source\\repos\\almc\\Debug\\new-lexer-test.txt", FROM_FILE);
	//Lexer* l = lexer_new("1. 0011 0xfff 0XFF 0o123 0O33 0b101010 0B1111 133.112   123.4e+5\n\r 63\n\r1e+15", FROM_CHAR_PTR);
	//Lexer* l = lexer_new("0.123 1.123e-2 1e+5 a\\nasd hello World!", FROM_CHAR_PTR);
	//Lexer* l = lexer_new("hello n world \"hello world!\" \'r\' \'\r\'", FROM_CHAR_PTR);
	//Lexer* l = lexer_new("C:\\Users\\HP\\source\\repos\\almc\\Debug\\token-table.txt", FROM_FILE);
	//Lexer* l = lexer_new("<= <<= >>= += \r\n== != ! &&", FROM_CHAR_PTR);
	Lexer* l = lexer_new("C:\\Users\\HP\\source\\repos\\almc\\Debug\\test4.txt", FROM_FILE);
	//Lexer* l = lexer_new("a a a a a a a aa a a a a a a a   a a a a a a a aa a a a a a a aa a a a  a aa a a a a", FROM_CHAR_PTR);
	Token** tokens = lex(l);

	/*double a = atof(tokens[0]->svalue);
	a = atof(tokens[1]->svalue);
	a = atof(tokens[2]->svalue);
	*/

	for (int i = 0; i < sbuffer_len(tokens); i++)
		printf("%s\n", token_tostr(tokens[i]));
	lexer_free(l);
	//run_tests();
	system("pause");
}