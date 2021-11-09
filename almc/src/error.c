#include "error.h"

#define TABSIZE 8

void report_warning(const char* message, SrcContext* context)
{
	printf("WARNING: %s,", message);
	if (context)
	{
		printf(" %s\n", src_context_tostr(context));
		char** lines = create_spell_lines(context);
		if (lines)
			printf("%s\n", lines[0]), printf("%s\n", lines[1]);
	}
	else
		printf("\n");
}

void report_error(const char* message, SrcContext* context)
{
	//todo: DEBUG macro does not exist
#ifndef ALMC_DEBUG
	printf("ERROR: %s,", message);
	if (context)
	{
		printf(" %s\n", src_context_tostr(context));
		char** lines = create_spell_lines(context);
		if (lines)
			printf("%s\n", lines[0]), printf("%s\n", lines[1]);
	}
	else
		printf("\n");
	assert(0);
#else
	assert(0);
	printf("ERROR: %s\n", message);
	system("pause");
	exit(1);
#endif
}

void report_error2(const char* message, SrcArea* area)
{
#ifndef ALMC_DEBUG
	printf("ERROR: %s,", message);
	if (area)
	{
		printf(" %s\n", src_area_tostr(area));
		char** lines = create_spell_lines(area->begins);
		if (lines)
			printf("%s\n", lines[0]), printf("%s\n", lines[1]);
	}
	else
		printf("\n");
	assert(0);
#else
	assert(0);
	printf("ERROR: %s\n", message);
	system("pause");
	exit(1);
#endif
}

char** create_spell_lines(SrcContext* context)
{
	FILE* source;
	fopen_s(&source, context->file, "r");
	if (!source)
		return NULL;

	uint32_t i = 0;
	char buffer[512];
	char** lines = (char*)calloc(2, sizeof(char*));

	while (fgets(buffer, 512, source))
	{
		//todo: extend for multiline error
		if (++i == context->line)
		{
			fclose(source);

			size_t len = 0;
			for (size_t j = 0; j < strlen(buffer); j++)
				len += buffer[j] == '\t' ? TABSIZE : 1;

			lines[0] = (char*)(calloc(len+1, sizeof(char)));
			lines[1] = (char*)(calloc(len+1, sizeof(char)));

			strcpy_s(lines[0], len + 1, buffer);

			for (size_t j = 0, s = 0; s < context->start + context->size; s++, j++)
			{
				if (lines[0][s] == '\t')
				{
					for (size_t z = 0; z < TABSIZE; z++)
						lines[1][j+z] = ' ';
					j += TABSIZE-1;
				}
				else if (s < context->start-1)
					lines[1][j] = ' ';
				else
					if (context->start + context->size - s > 1)
						lines[1][j] = '~';
			}
			return lines;
		}
	}
	return NULL;
}