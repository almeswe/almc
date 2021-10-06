#include "..\token.h"
#include "..\new-lexer\lexer2.h"

Lexer* lexer_new(const char* input, StreamType input_type)
{
	Lexer* l = new_s(Lexer, l);
	if (input_type == FROM_CHAR_PTR)
	{
		l->stream_origin = input;
		l->stream_size = strlen(l->stream_origin);
	}
	else
	{
		FILE* file;
		fopen_s(&file, input, "rb+");
		if (!file)
			report_error(frmt("Cannot open file: \'%s\'", input), NULL);
		else
		{
			l->curr_file = input;
			fseek(file, 0L, SEEK_END);
			l->stream_size = ftell(file);
			rewind(file);
			fread(l->stream_origin, sizeof(char), l->stream_size, file);
		}
	}

	l->stream = l->stream_origin;
	l->curr_line = l->curr_line_offset = 1;
	return l;
}

void lexer_free(Lexer* lexer)
{
	if (lexer)
	{
		free(lexer->stream_origin);
		free(lexer);
	}
}