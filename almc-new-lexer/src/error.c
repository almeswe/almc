#include "error.h"

void report_warning(const char* message, SrcContext* context)
{
	printf("WARNING: %s,", message);
	if (context)
		printf(" %s\n", src_context_tostr(context));
	else
		printf("\n");
}

void report_error(const char* message, SrcContext* context)
{
	//todo: DEBUG macro does not exist
#ifndef DEBUG
	printf("ERROR: %s,", message);
	if (context)
		printf(" %s\n", src_context_tostr(context));
	else
		printf("\n");
	assert(0);
#else
	printf("ERROR: %s\n", message);
	system("pause");
	exit(1);
#endif
}