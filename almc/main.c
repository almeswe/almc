#include "sb.h"
#include "assert.h"

typedef struct
{
	int a;
	long b;
	char c;
} A;

int main(int argc, char** argv)
{
	A* b = NULL;
	A a1 = *(A*)malloc(sizeof(A));
	a1.a = 1;
	a1.b = 1;
	a1.c = 1;
	A a2 = *(A*)malloc(sizeof(A));
	a2.a = 2;
	a2.b = 2;
	a2.c = 2;

	sbuffer_add(b, a1);
	sbuffer_add(b, a2);

	int* buf = NULL;

	for (int i = 0; i < 1024; i++)
	{
		sbuffer_add(buf, i);
		assert(buf[i] == i);
	}
	
	return 0;
}