#include "error.h"

enum
{
	TABSIZE = 0x08,
	ALMC_ERROR_FORECOLOR   = ALMC_CONSOLE_RED,
	ALMC_WARNING_FORECOLOR = ALMC_CONSOLE_YELLOW,
	ALMC_CONTENT_FORECOLOR = ALMC_CONSOLE_GRAY
};

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

			lines[0] = (char*)(calloc(len + 1, sizeof(char)));
			lines[1] = (char*)(calloc(len + 1, sizeof(char)));

			strcpy_s(lines[0], len + 1, buffer);

			for (size_t j = 0, s = 0; s < context->start + context->size; s++, j++)
			{
				if (lines[0][s] == '\t')
				{
					for (size_t z = 0; z < TABSIZE; z++)
						lines[1][j + z] = ' ';
					j += TABSIZE - 1;
				}
				else if (s < context->start - 1)
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

void free_spell_lines(char** lines)
{
	//for (size_t i = 0; i < sbuffer_len(lines); i++)
	//	sbuffer_free
}

void report_info_base(const char* header, const char* message, const char** spelling,
	any_context_to_str to_str_func, const void* any_context)
{
	// printing header [error, warning] and message
	printerr("%s: %s ", header, message);
	// printing context if there are any
	printerr("%s\n", any_context ? to_str_func(any_context) : "");
	// and finally printing spelling lines if there are any
	if (spelling)
		printerr("\n%s\n%s\n", spelling[0], spelling[1]);
}

void report_warning(const char* message,
	const SrcContext* context)
{
	report_info_base("Warning", message, create_spell_lines(
		context), src_context_tostr, context);
}

void report_warning2(const char* message,
	const SrcArea* area)
{
	report_info_base("Warning", message, create_spell_lines(
		area->begins), src_area_tostr, area);
}

void report_error(const char* message, 
	const SrcContext* context)
{
	report_info_base("Error", message, create_spell_lines(
		context), src_context_tostr, context);
	exit(1);
}

void report_error2(const char* message, 
	const SrcArea* area)
{
	report_info_base("Error", message, create_spell_lines(
		area->begins), src_area_tostr, area);
	exit(1);
}