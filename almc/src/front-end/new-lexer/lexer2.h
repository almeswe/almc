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

	const char* curr_file;
} Lexer;

Lexer* lexer_new(const char* input, StreamType input_type);
void lexer_free(Lexer* lexer);

Token** lex(Lexer* lexer);

#endif