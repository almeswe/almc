#ifndef ALMC_GLOBAL_LEXER_TEST_H
#define ALMC_GLOBAL_LEXER_TEST_H

#include <stdio.h>
#include <assert.h>
#include "..\..\..\src\front-end\front-end.h"

inline int compare_context(SrcContext* c, uint32_t start, uint32_t size, uint32_t line)
{
	return (c->start == start) && 
		   (c->size == size) && 
		   (c->line == line);
}

inline void lexer_lexeme_test()
{
	const char* file
		= "test\\test-cases\\lexer-test-cases\\LEXEME-TEST.txt";
	Lexer* lexer = lexer_new(file, FROM_FILE);
	Token** tokens = lex(lexer);
	assert(tokens[0]->type == TOKEN_PLUS);
	assert(tokens[1]->type == TOKEN_DASH);
	assert(tokens[2]->type == TOKEN_ASTERISK);
	assert(tokens[3]->type == TOKEN_SLASH);
	assert(tokens[4]->type == TOKEN_MODULUS);
	assert(tokens[5]->type == TOKEN_BAR);
	assert(tokens[6]->type == TOKEN_TILDE);
	assert(tokens[7]->type == TOKEN_CARET);
	assert(tokens[8]->type == TOKEN_EXCL_MARK);
	assert(tokens[9]->type == TOKEN_COMMA);
	assert(tokens[10]->type == TOKEN_DOT);
	assert(tokens[11]->type == TOKEN_COLON);
	assert(tokens[12]->type == TOKEN_SEMICOLON);
	assert(tokens[13]->type == TOKEN_ASSIGN);
	assert(tokens[14]->type == TOKEN_QUESTION);
	assert(tokens[15]->type == TOKEN_AMPERSAND);
	assert(tokens[16]->type == TOKEN_CL_PAREN);
	assert(tokens[17]->type == TOKEN_OP_PAREN);
	assert(tokens[18]->type == TOKEN_CL_BRACKET);
	assert(tokens[19]->type == TOKEN_OP_BRACKET);
	assert(tokens[20]->type == TOKEN_CL_BRACE);
	assert(tokens[21]->type == TOKEN_OP_BRACE);
	assert(tokens[22]->type == TOKEN_LEFT_ANGLE);
	assert(tokens[23]->type == TOKEN_RIGHT_ANGLE);
	assert(tokens[24]->type == TOKEN_ADD_ASSIGN);
	assert(tokens[25]->type == TOKEN_SUB_ASSIGN);
	assert(tokens[26]->type == TOKEN_MUL_ASSIGN);
	assert(tokens[27]->type == TOKEN_DIV_ASSIGN);
	assert(tokens[28]->type == TOKEN_MOD_ASSIGN);
	assert(tokens[29]->type == TOKEN_LSHIFT_ASSIGN);
	assert(tokens[30]->type == TOKEN_RSHIFT_ASSIGN);
	assert(tokens[31]->type == TOKEN_BW_OR_ASSIGN);
	assert(tokens[32]->type == TOKEN_BW_AND_ASSIGN);
	assert(tokens[33]->type == TOKEN_BW_XOR_ASSIGN);
	assert(tokens[34]->type == TOKEN_BW_NOT_ASSIGN);
	assert(tokens[35]->type == TOKEN_LG_OR);
	assert(tokens[36]->type == TOKEN_LG_EQ);
	assert(tokens[37]->type == TOKEN_LG_NEQ);
	assert(tokens[38]->type == TOKEN_LG_AND);
	assert(tokens[39]->type == TOKEN_LSHIFT);
	assert(tokens[40]->type == TOKEN_RSHIFT);
	assert(tokens[41]->type == TOKEN_LESS_EQ_THAN);
	assert(tokens[42]->type == TOKEN_GREATER_EQ_THAN);
	assert(tokens[43]->type == TOKEN_INC);
	assert(tokens[44]->type == TOKEN_DEC);
	assert(tokens[45]->type == TOKEN_ARROW);
	assert(tokens[46]->type == TOKEN_DOUBLE_DOT);
	assert(tokens[47]->type == TOKEN_TRIPLE_DOT);
	assert(tokens[48]->type == TOKEN_NAV_CURR_DIR);
	assert(tokens[49]->type == TOKEN_NAV_PREV_DIR);
	assert(tokens[50]->type == TOKEN_KEYWORD_AUTO);
	assert(tokens[51]->type == TOKEN_KEYWORD_BREAK);
	assert(tokens[52]->type == TOKEN_KEYWORD_CASE);
	assert(tokens[53]->type == TOKEN_KEYWORD_CHAR);
	assert(tokens[54]->type == TOKEN_KEYWORD_CONST);
	assert(tokens[55]->type == TOKEN_KEYWORD_CONTINUE);
	assert(tokens[56]->type == TOKEN_KEYWORD_DEFAULT);
	assert(tokens[57]->type == TOKEN_KEYWORD_ENUM);
	assert(tokens[58]->type == TOKEN_KEYWORD_ELIF);
	assert(tokens[59]->type == TOKEN_KEYWORD_EXTERN);
	assert(tokens[60]->type == TOKEN_KEYWORD_FOR);
	assert(tokens[61]->type == TOKEN_KEYWORD_FUNC);
	assert(tokens[62]->type == TOKEN_KEYWORD_FROM);
	assert(tokens[63]->type == TOKEN_KEYWORD_FALSE);
	assert(tokens[64]->type == TOKEN_KEYWORD_FLOAT32);
	assert(tokens[65]->type == TOKEN_KEYWORD_FLOAT64);
	assert(tokens[66]->type == TOKEN_KEYWORD_GOTO);
	assert(tokens[67]->type == TOKEN_KEYWORD_IF);
	assert(tokens[68]->type == TOKEN_KEYWORD_INT8);
	assert(tokens[69]->type == TOKEN_KEYWORD_INT16);
	assert(tokens[70]->type == TOKEN_KEYWORD_INT32);
	assert(tokens[71]->type == TOKEN_KEYWORD_INT64);
	assert(tokens[72]->type == TOKEN_KEYWORD_IMPORT);
	assert(tokens[73]->type == TOKEN_KEYWORD_REGISTER);
	assert(tokens[74]->type == TOKEN_KEYWORD_RETURN);
	assert(tokens[75]->type == TOKEN_KEYWORD_STRING);
	assert(tokens[76]->type == TOKEN_KEYWORD_STATIC);
	assert(tokens[77]->type == TOKEN_KEYWORD_STRUCT);
	assert(tokens[78]->type == TOKEN_KEYWORD_SWITCH);
	assert(tokens[79]->type == TOKEN_KEYWORD_SIZEOF);
	assert(tokens[80]->type == TOKEN_KEYWORD_TRUE);
	assert(tokens[81]->type == TOKEN_KEYWORD_TYPEOF);
	assert(tokens[82]->type == TOKEN_KEYWORD_UNION);
	assert(tokens[83]->type == TOKEN_KEYWORD_UINT8);
	assert(tokens[84]->type == TOKEN_KEYWORD_UINT16);
	assert(tokens[85]->type == TOKEN_KEYWORD_UINT32);
	assert(tokens[86]->type == TOKEN_KEYWORD_UINT64);
	assert(tokens[87]->type == TOKEN_KEYWORD_LET);
	assert(tokens[88]->type == TOKEN_KEYWORD_LABEL);
	assert(tokens[89]->type == TOKEN_KEYWORD_VOID);
	assert(tokens[90]->type == TOKEN_KEYWORD_VOLATILE);
	assert(tokens[91]->type == TOKEN_KEYWORD_WHILE);
	assert(tokens[92]->type == TOKEN_KEYWORD_DO);
	assert(tokens[93]->type == TOKEN_KEYWORD_ELSE);
	lexer_free(lexer);
}

inline void lexer_comment_test()
{
	const char* file =
		"test\\test-cases\\lexer-test-cases\\COMMENTS-TEST.txt";
	Lexer* lexer = lexer_new(file, FROM_FILE);
	Token** tokens = lex(lexer);
	// comment1, comment3, eof
	assert(sbuffer_len(tokens) == 3);
	assert(tokens[0]->type == TOKEN_IDNT);
	assert(strcmp(tokens[0]->svalue, "comment1") == 0);
	assert(tokens[1]->type == TOKEN_IDNT);
	assert(strcmp(tokens[1]->svalue, "comment3") == 0);
	lexer_free(lexer);
}

inline void lexer_recognition_test()
{
	const char* file =
		"test\\test-cases\\lexer-test-cases\\RECOGNITION-TEST.txt";
	Lexer* lexer = lexer_new(file, FROM_FILE);
	Token** tokens = lex(lexer);
	assert(sbuffer_len(tokens) == 31);
	assert(tokens[0]->type == TOKEN_INUM);
	assert(strcmp(tokens[0]->svalue, "1E+5") == 0);
	assert(tokens[1]->type == TOKEN_INUM);
	assert(strcmp(tokens[1]->svalue, "123e-5") == 0);
	assert(tokens[2]->type == TOKEN_FNUM);
	assert(strcmp(tokens[2]->svalue, "1.123e-1") == 0);
	assert(tokens[3]->type == TOKEN_FNUM);
	assert(strcmp(tokens[3]->svalue, "1.33E+3") == 0);

	assert(tokens[4]->type == TOKEN_INUM);
	assert(strcmp(tokens[4]->svalue, "255") == 0);
	assert(tokens[5]->type == TOKEN_INUM);
	assert(strcmp(tokens[5]->svalue, "0") == 0);
	assert(tokens[6]->type == TOKEN_FNUM);
	assert(strcmp(tokens[6]->svalue, "1.11") == 0);
	assert(tokens[7]->type == TOKEN_FNUM);
	assert(strcmp(tokens[7]->svalue, "0.00333") == 0);

	assert(tokens[8]->type == TOKEN_INUM);
	assert(strcmp(tokens[8]->svalue, "0xff") == 0);
	assert(tokens[9]->type == TOKEN_INUM);
	assert(strcmp(tokens[9]->svalue, "0XFF") == 0);
	assert(tokens[10]->type == TOKEN_INUM);
	assert(strcmp(tokens[10]->svalue, "0x123") == 0);

	assert(tokens[11]->type == TOKEN_INUM);
	assert(strcmp(tokens[11]->svalue, "0b0011") == 0);
	assert(tokens[12]->type == TOKEN_INUM);
	assert(strcmp(tokens[12]->svalue, "0B0111") == 0);
	assert(tokens[13]->type == TOKEN_INUM);
	assert(strcmp(tokens[13]->svalue, "0b00001") == 0);

	assert(tokens[14]->type == TOKEN_INUM);
	assert(strcmp(tokens[14]->svalue, "0o123") == 0);
	assert(tokens[15]->type == TOKEN_INUM);
	assert(strcmp(tokens[15]->svalue, "0O333") == 0);
	assert(tokens[16]->type == TOKEN_INUM);
	assert(strcmp(tokens[16]->svalue, "0o222") == 0);

	assert(tokens[17]->type == TOKEN_STRING);
	assert(strcmp(tokens[17]->svalue, "hello world!") == 0);
	assert(tokens[18]->type == TOKEN_STRING);
	assert(strcmp(tokens[18]->svalue, "hello world\'\"\\") == 0);

	assert(tokens[19]->type == TOKEN_CHARACTER);
	assert(tokens[19]->cvalue == '\n');
	assert(tokens[20]->type == TOKEN_CHARACTER);
	assert(tokens[20]->cvalue == '\r');
	assert(tokens[21]->type == TOKEN_CHARACTER);
	assert(tokens[21]->cvalue == '\'');
	assert(tokens[22]->type == TOKEN_CHARACTER);
	assert(tokens[22]->cvalue == '\"');
	assert(tokens[23]->type == TOKEN_CHARACTER);
	assert(tokens[23]->cvalue == '\0');
	assert(tokens[24]->type == TOKEN_CHARACTER);
	assert(tokens[24]->cvalue == '\t');
	assert(tokens[25]->type == TOKEN_CHARACTER);
	assert(tokens[25]->cvalue == '\f');
	assert(tokens[26]->type == TOKEN_CHARACTER);
	assert(tokens[26]->cvalue == '\a');
	assert(tokens[27]->type == TOKEN_CHARACTER);
	assert(tokens[27]->cvalue == '\\');
	assert(tokens[28]->type == TOKEN_CHARACTER);
	assert(tokens[28]->cvalue == '\b');
	assert(tokens[29]->type == TOKEN_CHARACTER);
	assert(tokens[29]->cvalue == '\v');
	lexer_free(lexer);
}

inline void lexer_src_context_test()
{
	const char* file =
		"test\\test-cases\\lexer-test-cases\\SRC-CONTEXT-TEST.txt";
	Lexer* lexer = lexer_new(file, FROM_FILE);
	Token** tokens = lex(lexer);
	int a = sbuffer_len(tokens);
	assert(compare_context(tokens[125]->context, 21, 3, 18)); // 'e'
	assert(compare_context(tokens[140]->context, 39, 6, 19)); //buffer
	assert(compare_context(tokens[72]->context, 21, 42, 13)); //string
	lexer_free(lexer);
}

inline void lexer_run_tests()
{
	lexer_lexeme_test();
	lexer_comment_test();
	lexer_recognition_test();
	lexer_src_context_test();
}

#endif