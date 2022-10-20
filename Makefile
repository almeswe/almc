all:
	gcc -o almc src/*.c src/utils/*.c src/utils/xmemory/*.c src/utils/data-structures/*.c  src/front-end/*.c src/back-end/x86_64/*.c -g