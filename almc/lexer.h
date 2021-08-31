#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <ctype.h>
#include <stdint.h>
#include <string.h>
#include "sbuffer.h"

#ifndef LEXER_STREAM_MACROS
#define LEXER_STREAM_MACROS

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
#define close_stream(lex) ((lex->stream_type == STREAM_FILE) ? (fclose(lex->file_stream)) : (free(lex->char_stream)))

#define matchc(lex, ch) (get_curr_char(lex) == ch)
#define matchc_in(lex, c1, c2) ((get_curr_char(lex)) >= (c1) && (get_curr_char(lex)) <= (c2))

//todo: remove assert here
#define check_overflow() ((value < prev_value) ? assert(!"Overflow occured!") : assert(1)), (prev_value = value)
#define get_token_not_in_dec_format(lex) (matchc(lex, 'x') ? get_hex_num_token(lex) : get_bin_num_token(lex))
//#define matcht(lex, type) (g)

#endif

#define EXT_CHARS 19
#define EXT_CHARS_IN_TOKEN_ENUM_OFFSET TOKEN_RIGHT_ANGLE + 1

#define CHARS 25
#define CHARS_IN_TOKEN_ENUM_OFFSET 0

#define KEYWORDS 32
#define KEYWORD_IN_TOKEN_ENUM_OFFSET TOKEN_IDNT + 1

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
		double fvalue;
		uint64_t ivalue;
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
	size_t prev_line;
	size_t prev_line_offset;
} LexerBackupData;

typedef struct
{
	Token* tokens;
	uint32_t token_index;

	uint32_t curr_line;
	uint32_t curr_line_offset;

	LexerBackupData backup;

	InputStreamType stream_type;
	union
	{
		FILE* file_stream;
		const char* char_stream;
	};
} Lexer;

Lexer* lexer_new(const char* src, InputStreamType type);
Token* token_new(TokenType type, SrcContext* context);
SrcContext* src_context_new(const char* file, size_t sym, size_t size, size_t line);

Token* lexer_get_tokens(Lexer* lex);

void multi_line_comment(Lexer* lex);
void single_line_comment(Lexer* lex);

char get_next_char(Lexer* lex);
void unget_curr_char(Lexer* lex);
Token* get_next_token();
Token* get_dec_num_token(Lexer* lex);
Token* get_bin_num_token(Lexer* lex);
Token* get_hex_num_token(Lexer* lex);
Token* get_dec_fnum_token(Lexer* lex, uint64_t base_inum, size_t size);
Token* get_idnt_token(Lexer* lex);
Token* get_char_token(Lexer* lex);
Token* get_string_token(Lexer* lex);
Token* get_keychar_token(Lexer* lex, int order);
Token* get_keyword_token(Lexer* lex, int order);