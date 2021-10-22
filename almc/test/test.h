#ifndef ALMC_TESTS_H
#define ALMC_TESTS_H
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <assert.h>
#include "..\src\front-end\front-end.h"

#include "..\test\test-cases\lexer-test-cases\lexer-test-cases.h"
#include "..\test\test-cases\parser-test-cases\ast-expr-eval-test.h"
#include "..\test\test-cases\parser-test-cases\parser-ast-manual-tests\parser-manual-test.h"
#include "..\test\test-cases\parser-test-cases\parser-ast-tests\import-tests\import-stmt-tests.h"

void run_tests();

#endif