almc: src/main2.c
	gcc -o almc src/main2.c src/error.c src/utils/context.c src/utils/common.c src/utils/xmemory/xmemory.c src/utils/data-structures/sbuffer.c src/front-end/token.c src/front-end/lexer.c -g