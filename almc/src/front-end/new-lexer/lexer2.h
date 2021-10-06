#ifndef ALMC_NEW_LEXER_H
#define ALMC_NEW_LEXER_H
#define NEW_LEXER_IN_USE	

#include <stdio.h>
#include <ctype.h>
#include <stdint.h>
#include <string.h>

#include "..\token.h"
#include "..\..\utils\sbuffer.h"
#include "..\..\..\src\error.h"

#define eos(lexer) \
	((lexer->stream_origin - lexer->stream) >= lexer->stream_size)

#define curr_char_fits(lexer) \
	(((lexer->stream_origin - lexer->stream) >= 0) && (eos(lexer)))

typedef enum StreamType
{
	FROM_FILE,
	// used especially for lexer debugging
	FROM_CHAR_PTR
} StreamType;

typedef struct Lexer
{
	char* stream;
	char* stream_origin;
	uint32_t stream_size;
		
	uint32_t curr_line;
	uint32_t curr_line_offset;

	//for proper use of unget_curr_char
	uint32_t prev_line;
	uint32_t prev_line_offset;

	const char* curr_file;
} Lexer2;

Lexer2* lexer2_new(const char* input, StreamType input_type);
void lexer2_free(Lexer2* lexer);

Token** lex(Lexer2* lexer);

int32_t get_next_char(Lexer2* lexer);
int32_t get_curr_char(Lexer2* lexer);
int32_t unget_curr_char(Lexer2* lexer);

#endif