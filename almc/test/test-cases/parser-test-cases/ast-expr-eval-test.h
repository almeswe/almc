#ifndef ALMC_GLOBAL_AST_EXPR_EVAL_TEST_H
#define ALMC_GLOBAL_AST_EXPR_EVAL_TEST_H

#include <stdio.h>
#include <assert.h>
#include "..\..\..\src\front-end\front-end.h"

inline void ast_expr_eval_run_tests()
{
	// Tests for checking the ast creation for simple expressions via evaluation
#define parser_eval_test_case_free(lexer, parser)    \
	lexer_free(lexer);  \
	free(parser);       \

#define parser_eval_test_case_init(expr, str_expr)  \
	l = lexer_new(str_expr, FROM_CHAR_PTR);\
	p = parser_new(lex(l));						    \
	ast = parse_expr(p);						    \
	assert((expr) == eval_expr(ast)); 			    \
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

#endif