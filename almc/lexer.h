#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <ctype.h>
#include <stdint.h>
#include <string.h>

#include "error.h"
#include "common.h"
#include "context.h"
#include "sbuffer.h"

#ifndef LEXER_H
#define LEXER_H

#define get__next_char_cstream(lex) (*(++lex->char_stream))
#define get__next_char_fstream(lex) (fgetc(lex->file_stream))
#define get__next_char(lex) ((lex->stream_type == STREAM_FILE) ? get__next_char_fstream(lex) : get__next_char_cstream(lex))

#define get__curr_char_cstream(lex) (*lex->char_stream)
#define get__curr_char_fstream(lex) (fgetc_ext(lex->file_stream))

#define unget__curr_char_fstream(lex) (ungetc(get__curr_char_fstream(lex), lex->file_stream))
#define unget__curr_char_cstream(lex) (--lex->char_stream)
#define unget__curr_char(lex) ((lex->stream_type == STREAM_FILE) ? unget__curr_char_fstream(lex) : unget__curr_char_cstream(lex))

#define get_curr_char(lex) ((lex->stream_type == STREAM_FILE) ? get__curr_char_fstream(lex) : get__curr_char_cstream(lex))

#define is_stream_empty(lex) ((lex->stream_type == STREAM_FILE) ? (feof(lex->file_stream)) : (*lex->char_stream == '\0'))
#define close_stream(lex) ((lex->stream_type == STREAM_FILE) ? (fclose(lex->file_stream)) : 0)

#define matchc(lex, ch) (get_curr_char(lex) == ch)
#define matchc_in(lex, c1, c2) ((get_curr_char(lex)) >= (c1) && (get_curr_char(lex)) <= (c2))

// Initializes all needed variables for sft_with_overflow macro 
#define sft_init_vars(max_sfts, format) char shifts = 0; char max_shifts = max_sfts; const char base = format; char met = 0
#define sft_zero_check(ch) (!met && ch == '0' ? max_shifts++ : met++)
#define sft_with_overflow(a, c) shifts++, ((shifts <= max_shifts) ? a = a * base : lexer_raise_int_overflow_error(c))
#define add_init_vars(a) uint64_t prev_value = a
#define add_with_overflow(a, b, c) ((a <= (ULLONG_MAX - (b)) && (a >= prev_value)) ? (a = a + (b), prev_value = a) : lexer_raise_int_overflow_error(c))

#define EXT_CHARS 19
#define EXT_CHARS_IN_TOKEN_ENUM_OFFSET TOKEN_RIGHT_ANGLE + 1
	
#define CHARS 25
#define CHARS_IN_TOKEN_ENUM_OFFSET 0

#define KEYWORDS 32
#define KEYWORD_IN_TOKEN_ENUM_OFFSET TOKEN_IDNT + 1

#define TOKEN_TYPE_STR(type) (type < (KEYWORDS + CHARS + EXT_CHARS) && type >= 0) ? tokens_str[type] : tokens_str[0]

static const char* tokens_str[] = {
	"TOKEN_PLUS",
	"TOKEN_DASH",
	"TOKEN_ASTERISK",
	"TOKEN_SLASH,",
	"TOKEN_MODULUS",
	"TOKEN_BAR",
	"TOKEN_TILDE",
	"TOKEN_CARET",
	"TOKEN_S_QUOTE",
	"TOKEN_D_QUOTE",
	"TOKEN_EXCL_MARK",
	"TOKEN_COMMA",
	"TOKEN_COLON",
	"TOKEN_SEMICOLON",
	"TOKEN_ASSIGN",
	"TOKEN_QUESTION",
	"TOKEN_AMPERSAND",
	"TOKEN_CL_PAREN",
	"TOKEN_OP_PAREN",
	"TOKEN_CL_BRACKET",
	"TOKEN_OP_BRACKET",
	"TOKEN_CL_BRACE",
	"TOKEN_OP_BRACE",
	"TOKEN_LEFT_ANGLE",
	"TOKEN_RIGHT_ANGLE",
	
	"TOKEN_ADD_ASSIGN",
	"TOKEN_SUB_ASSIGN",
	"TOKEN_MUL_ASSIGN",
	"TOKEN_DIV_ASSIGN",
	"TOKEN_MOD_ASSIGN",
	"TOKEN_LSHIFT_ASSIGN",
	"TOKEN_RSHIFT_ASSIGN",
	
	"TOKEN_BW_NOT_ASSIGN",
	"TOKEN_BW_OR_ASSIGN",
	"TOKEN_BW_AND_ASSIGN",
	"TOKEN_BW_XOR_ASSIGN",
	
	"TOKEN_LG_OR",
	"TOKEN_LG_NEQ",
	"TOKEN_LG_EQ",
	"TOKEN_LG_AND",
	
	"TOKEN_LSHIFT",
	"TOKEN_RSHIFT",
	"TOKEN_INC",
	"TOKEN_DEC,",
	
	"TOKEN_STRING",
	"TOKEN_CHARACTER",
	"TOKEN_INUM",
	"TOKEN_FNUM",
	"TOKEN_IDNT",
	
	"TOKEN_KEYWORD_AUTO",
	"TOKEN_KEYWORD_BREAK",
	"TOKEN_KEYWORD_CASE",
	"TOKEN_KEYWORD_CHAR",
	"TOKEN_KEYWORD_CONST",
	"TOKEN_KEYWORD_CONTINUE",
	"TOKEN_KEYWORD_DEFAULT",
	"TOKEN_KEYWORD_DOUBLE",
	"TOKEN_KEYWORD_ENUM",
	"TOKEN_KEYWORD_EXTERN",
	"TOKEN_KEYWORD_FLOAT",
	"TOKEN_KEYWORD_FOR",
	"TOKEN_KEYWORD_GOTO",
	"TOKEN_KEYWORD_IF",
	"TOKEN_KEYWORD_INT",
	"TOKEN_KEYWORD_LONG",
	"TOKEN_KEYWORD_REGISTER",
	"TOKEN_KEYWORD_RETURN",
	"TOKEN_KEYWORD_SHORT",
	"TOKEN_KEYWORD_SIGNED",
	"TOKEN_KEYWORD_SIZEOF",
	"TOKEN_KEYWORD_STATIC",
	"TOKEN_KEYWORD_STRUCT",
	"TOKEN_KEYWORD_SWITCH",
	"TOKEN_KEYWORD_TYPEOF",
	"TOKEN_KEYWORD_UNION",
	"TOKEN_KEYWORD_UNSIGNED",
	"TOKEN_KEYWORD_VOID",
	"TOKEN_KEYWORD_VOLATILE",
	"TOKEN_KEYWORD_WHILE",
	"TOKEN_KEYWORD_DO",
	"TOKEN_KEYWORD_ELSE",
};

typedef enum
{
	TOKEN_PLUS,
	TOKEN_DASH,
	TOKEN_ASTERISK,
	TOKEN_SLASH,	
	TOKEN_MODULUS,
	TOKEN_BAR,
	TOKEN_TILDE,
	TOKEN_CARET,
	TOKEN_S_QUOTE,
	TOKEN_D_QUOTE,
	TOKEN_EXCL_MARK,
	TOKEN_COMMA,
	TOKEN_COLON,
	TOKEN_SEMICOLON,
	TOKEN_ASSIGN,
	TOKEN_QUESTION,
	TOKEN_AMPERSAND,
	TOKEN_CL_PAREN,
	TOKEN_OP_PAREN,
	TOKEN_CL_BRACKET,
	TOKEN_OP_BRACKET,
	TOKEN_CL_BRACE,
	TOKEN_OP_BRACE,
	TOKEN_LEFT_ANGLE,
	TOKEN_RIGHT_ANGLE,

	TOKEN_ADD_ASSIGN,
	TOKEN_SUB_ASSIGN,
	TOKEN_MUL_ASSIGN,
	TOKEN_DIV_ASSIGN,
	TOKEN_MOD_ASSIGN,
	TOKEN_LSHIFT_ASSIGN,
	TOKEN_RSHIFT_ASSIGN,

	TOKEN_BW_NOT_ASSIGN,
	TOKEN_BW_OR_ASSIGN,
	TOKEN_BW_AND_ASSIGN,
	TOKEN_BW_XOR_ASSIGN,

	TOKEN_LG_OR,
	TOKEN_LG_NEQ,
	TOKEN_LG_EQ,
	TOKEN_LG_AND,
	
	TOKEN_LSHIFT,
	TOKEN_RSHIFT,
	TOKEN_INC,
	TOKEN_DEC,	

	TOKEN_STRING,
	TOKEN_CHARACTER,
	TOKEN_INUM,
	TOKEN_FNUM,
	TOKEN_IDNT,

	TOKEN_KEYWORD_AUTO,
	TOKEN_KEYWORD_BREAK,
	TOKEN_KEYWORD_CASE,
	TOKEN_KEYWORD_CHAR,
	TOKEN_KEYWORD_CONST,
	TOKEN_KEYWORD_CONTINUE,
	TOKEN_KEYWORD_DEFAULT,
	TOKEN_KEYWORD_DOUBLE,
	TOKEN_KEYWORD_ENUM,
	TOKEN_KEYWORD_EXTERN,
	TOKEN_KEYWORD_FLOAT,
	TOKEN_KEYWORD_FOR,
	TOKEN_KEYWORD_GOTO,
	TOKEN_KEYWORD_IF,
	TOKEN_KEYWORD_INT,
	TOKEN_KEYWORD_LONG,
	TOKEN_KEYWORD_REGISTER,
	TOKEN_KEYWORD_RETURN,
	TOKEN_KEYWORD_SHORT,
	TOKEN_KEYWORD_SIGNED,
	TOKEN_KEYWORD_SIZEOF,
	TOKEN_KEYWORD_STATIC,
	TOKEN_KEYWORD_STRUCT,
	TOKEN_KEYWORD_SWITCH,
	TOKEN_KEYWORD_TYPEOF,
	TOKEN_KEYWORD_UNION,
	TOKEN_KEYWORD_UNSIGNED,
	TOKEN_KEYWORD_VOID,
	TOKEN_KEYWORD_VOLATILE,
	TOKEN_KEYWORD_WHILE,
	TOKEN_KEYWORD_DO,
	TOKEN_KEYWORD_ELSE,
} TokenType;

typedef enum
{
	FORMAT_DEC,
	FORMAT_BIN,
	FORMAT_OCT,
	FORMAT_HEX,
} NumericFormat;

typedef struct
{
	union
	{
		double fvalue;
		uint64_t ivalue;
		char char_value;
		const char* str_value;
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
	uint32_t prev_line;
	uint32_t prev_line_offset;
} LexerBackupData;

typedef struct
{
	Token* tokens;
	uint32_t token_index;

	uint32_t curr_line;
	uint32_t curr_line_offset;

	const char* curr_file;
	LexerBackupData backup;

	InputStreamType stream_type;
	union
	{
		FILE* file_stream;
		const char* char_stream;
	};
} Lexer;

#endif //LEXER_H

Lexer* lexer_new(const char* src, InputStreamType type);
Token* token_new(TokenType type, SrcContext* context);

Token* lexer_get_tokens(Lexer* lex);

void multi_line_comment(Lexer* lex);
void single_line_comment(Lexer* lex);

char get_next_char(Lexer* lex);
void unget_curr_char(Lexer* lex);
Token* get_next_token();

int get_tokens_format(Lexer* lex);
Token* get_num_token(Lexer* lex);
Token* get_bin_num_token(Lexer* lex);
Token* get_hex_num_token(Lexer* lex);
Token* get_oct_num_token(Lexer* lex);
Token* get_dec_num_token(Lexer* lex);
Token* get_dec_fnum_token(Lexer* lex, uint64_t base_inum, uint32_t size);

Token* get_idnt_token(Lexer* lex);
Token* get_char_token(Lexer* lex);
Token* get_string_token(Lexer* lex);
Token* get_keychar_token(Lexer* lex, int order);
Token* get_keyword_token(Lexer* lex, int order);
char is_escape_sequence(Lexer* lex);