#ifndef _ALMC_NEW_LEXER_H
#define _ALMC_NEW_LEXER_H

#include <stdio.h>
#include <ctype.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>

#include "type.h"
#include "token.h"

#include "../error.h"
#include "../utils/common.h"
#include "../utils/context.h"
#include "../utils/data-structures/sbuffer.h"
#include "../utils/data-structures/str-builder.h"

typedef enum StreamType {
	FROM_FILE,
	FROM_CHAR_PTR
} StreamType;

typedef enum NumericFormat {
	FORMAT_DEC,
	FORMAT_BIN,
	FORMAT_OCT,
	FORMAT_HEX,
} NumericFormat;

typedef struct Lexer {
	// pointer that points to the current character
	// in the current char stream
	char* stream;
	// pointer to the non-shifted character stream
	// needed only for proper releasing memory after the lexical analysis
	char* stream_origin;
	size_t stream_size;
	
	// current line number
	size_t curr_line;
	// offset in the current number
	size_t curr_line_offset;

	// two fields above have the same sematcics
	// as the previous two, but vise-versa
	// for proper use of unget_curr_char
	size_t prev_line;
	size_t prev_line_offset;

	StreamType input;

	char* curr_file;
} Lexer;

enum LexerMetrics {
	// the definitions above are the same for all other
	// lexer metrics

	// means the count of reserved characters like:
	// *, |, ^, ~ ...
	CHARS = 26,
	// means the start offset in the global token enum
	CHARS_OFFSET = 0,

	// CoMPounded chars, means the strings like: 
	// +=, >>=, <<, || ...
	CMP_CHARS = 24,
	CMP_CHARS_OFFSET = TOKEN_ADD_ASSIGN,

	// just predefined keywords like:
	// for, fnc, while, i32 ...
	KEYWORDS = 41,
	KEYWORD_IN_TOKEN_ENUM_OFFSET = TOKEN_KEYWORD_BREAK
};

Lexer* lexer_new(char* input, StreamType input_type);
void lexer_free(Lexer* lexer);

Token** lex(Lexer* lexer);

bool check_comment(Lexer* lexer);
void sline_comment(Lexer* lexer);
void mline_comment(Lexer* lexer);

ssize_t get_next_char(Lexer* lexer);
ssize_t get_curr_char(Lexer* lexer);
ssize_t unget_curr_char(Lexer* lexer);

Token* get_eof_token(Lexer* lexer);
Token* get_num_token(Lexer* lexer);
Token* get_bin_num_token(Lexer* lexer);
Token* get_hex_num_token(Lexer* lexer);
Token* get_oct_num_token(Lexer* lexer);
Token* get_dec_num_token(Lexer* lexer);
Token* get_dec_fnum_token(Lexer* lexer, char* buffer, size_t size);
Token* get_dec_expnum_token(Lexer* lexer, char* buffer, size_t size, char is_float);

Token* get_idnt_token(Lexer* lexer);
Token* get_char_token(Lexer* lexer);
Token* get_string_token(Lexer* lexer);
Token* get_keychar_token(Lexer* lexer, int order);
Token* get_keyword_token(Lexer* lexer, int order);

#endif // _ALMC_NEW_LEXER_H