#include "list.h"
#include "assert.h"

typedef struct
{
	int a;
	long b;
	char c;
} A;

int main(int argc, char** argv)
{
	int* b = NULL;
	for (int i = 0; i < 123123; i++)
	{
		sbuffer_add(b, i);
		//assert(b[i], i);
	}

	return 0;
}