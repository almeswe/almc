#define _CRT_SECURE_NO_WARNINGS

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "sbuffer.h"

//todo: is needed?	
#ifndef LEXER_STREAM_MACROS
#define LEXER_STREAM_MACROS

#define get__next_char_cstream(lex) (*(++lex->char_stream))
#define get__next_char_fstream(lex) (fgetc(lex->file_stream))
#define get__next_char(lex) ((lex->type == STREAM_FILE) ? get__next_char_fstream(lex) : get__next_char_cstream(lex))

#define get__curr_char_cstream(lex) (*lex->char_stream)
#define get__curr_char_fstream(lex) (fseek(lex->file_stream, 0, SEEK_CUR))

#define get_curr_char(lex) ((lex->type == STREAM_FILE) ? get__curr_char_fstream(lex) : get__curr_char_cstream(lex))
#define is_stream_empty(lex) ((lex->type == STREAM_FILE) ? (feof(lex->file_stream)) : (*lex->char_stream == '\0'))

#endif

typedef enum
{
	TOKEN_NUM,
	TOKEN_IDNT,
} TokenType;

typedef struct
{
	size_t size;
	size_t line;
	size_t start;
	const char* file;
} SrcContext;

typedef struct
{
	union
	{
		uint64_t int_value;
		const char* op_value;
		const char* idnt_value;
	};
	TokenType type;
	SrcContext* context;
} Token;

typedef enum
{
	STREAM_FILE,
	STREAM_CHAR_PTR
} InputStreamType;

typedef struct
{
	Token* tokens;
	uint32_t token_index;

	uint32_t curr_line;
	uint32_t curr_line_offset;

	InputStreamType type;
	union
	{
		const char* char_stream;
		FILE* file_stream;
	};
} Lexer;

Lexer* lexer_new(const char* src, InputStreamType type);
Token* token_new(TokenType type, SrcContext* context);
SrcContext* src_context_new(const char* file, size_t sym, size_t size, size_t line);

Token* lexer_get_tokens(Lexer* lex);

void multi_line_comment(Lexer* lex);
void single_line_comment(Lexer* lex);

char get_next_char(Lexer* lex);
Token* get_next_token();
Token* get_num_token(Lexer* lex);
Token* get_idnt_token(Lexer* lex);

int fgetc_ext(FILE* file);