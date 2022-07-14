CC=gcc
DEBUG=-g

main: src/main2.c
	gcc -o almc src/front-end/visitor.c src/front-end/ast-evaluator.c src/front-end/ast-printer.c src/utils/os.c src/main2.c src/error.c src/utils/context.c src/utils/common.c src/utils/xmemory/xmemory.c src/utils/data-structures/sbuffer.c src/front-end/token.c src/front-end/lexer.c src/front-end/parser.c src/front-end/type-checker.c src/front-end/table.c src/front-end/ast.c src/front-end/type.c -g

front-end: $(wildcard src/front-end/*.o)
	$(CC) -c $(DEBUG)