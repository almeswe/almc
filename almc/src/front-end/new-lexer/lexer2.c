#include "lexer2.h"

Lexer2* lexer2_new(const char* input, StreamType input_type)
{
	Lexer2* l = new_s(Lexer2, l);
	if (input_type == FROM_CHAR_PTR)
	{
		l->stream_origin = input;
		l->stream_size = strlen(l->stream_origin);
	}
	else
	{
		FILE* file;
		fopen_s(&file, input, "r");
		if (!file)
			report_error(frmt("Cannot open file: \'%s\'", input), NULL);
		else
		{
			l->curr_file = input;
			fseek(file, 0L, SEEK_END);
			l->stream_size = ftell(file);
			rewind(file);
			l->stream_origin = newc_s(char, l->stream_origin, l->stream_size);
			fread(l->stream_origin, sizeof(char), l->stream_size, file);
			fclose(file);
		}
	}

	l->stream = l->stream_origin;
	l->curr_line = l->curr_line_offset = 1;
	return l;
}

void lexer2_free(Lexer2* lexer)
{
	if (lexer)
	{
		free(lexer->stream_origin);
		free(lexer);
	}
}

int32_t get_next_char(Lexer2* lexer)
{
	if (eos(lexer)) 
		return EOF;
	else
	{
		int32_t ch;
		switch (ch = *(lexer->stream++)) 
		{
		case '\r':
			lexer->prev_line_offset =
				lexer->curr_line_offset;
			lexer->curr_line_offset = 1;
			break;
		case '\n':
			lexer->prev_line = lexer->curr_line;
			lexer->curr_line += 1;
			break;
		default:
			lexer->curr_line_offset++;
			break;
		}
		return ch;
	}
}

int32_t get_curr_char(Lexer2* lexer)
{
	return curr_char_fits(lexer) ?
		*lexer->stream : EOF;
}

int32_t unget_curr_char(Lexer2* lexer)
{

}