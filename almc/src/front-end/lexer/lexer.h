#ifndef ALMC_NEW_LEXER_H
#define ALMC_NEW_LEXER_H
#define NEW_LEXER_IN_USE	

#include <stdio.h>
#include <ctype.h>
#include <stdint.h>
#include <string.h>

#include "..\lexer\token.h"
#include "..\..\..\src\error.h"
#include "..\..\..\src\utils\data-structures\sbuffer.h"

#define EXT_CHARS 22
#define EXT_CHARS_IN_TOKEN_ENUM_OFFSET TOKEN_RIGHT_ANGLE + 1

#define CHARS 26
#define CHARS_IN_TOKEN_ENUM_OFFSET 0

#define KEYWORDS 43
#define KEYWORD_IN_TOKEN_ENUM_OFFSET TOKEN_IDNT + 1

typedef enum StreamType
{
	FROM_FILE,
	// used especially for lexer debugging
	FROM_CHAR_PTR
} StreamType;

typedef enum NumericFormat
{
	FORMAT_DEC,
	FORMAT_BIN,
	FORMAT_OCT,
	FORMAT_HEX,
} NumericFormat;

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

	StreamType input;

	const char* curr_file;
} Lexer;

Lexer* lexer_new(const char* input, StreamType input_type);
void lexer_free(Lexer* lexer);

Token** lex(Lexer* lexer);

int check_comment(Lexer* lexer);
void mult_comment(Lexer* lexer);
void sngl_comment(Lexer* lexer);

int32_t get_next_char(Lexer* lexer);
int32_t get_curr_char(Lexer* lexer);
int32_t unget_curr_char(Lexer* lexer);

Token* get_eof_token(Lexer* lexer);
Token* get_num_token(Lexer* lexer);
Token* get_bin_num_token(Lexer* lexer);
Token* get_hex_num_token(Lexer* lexer);
Token* get_oct_num_token(Lexer* lexer);
Token* get_dec_num_token(Lexer* lexer);
Token* get_dec_fnum_token(Lexer* lexer, char* buffer, uint32_t size);
Token* get_dec_expnum_token(Lexer* lexer, char* buffer, uint32_t size, char is_float);

Token* get_idnt_token(Lexer* lexer);
Token* get_char_token(Lexer* lexer);
Token* get_string_token(Lexer* lexer);
Token* get_keychar_token(Lexer* lexer, int order);
Token* get_keyword_token(Lexer* lexer, int order);

#endif