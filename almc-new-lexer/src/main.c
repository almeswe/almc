//#include "../test/test.h"
#include "..\src\front-end\new-lexer\lexer.h"

void main(int argc, char** argv)
{
	//Lexer* l = lexer_new("123 1e+4 1.123 1.11e-5", FROM_CHAR_PTR);
	Lexer* l = lexer_new("1.123 1.11e-5", FROM_CHAR_PTR);
	lex(l);
	//run_tests();
	system("pause");
}