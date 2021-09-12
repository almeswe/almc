#ifndef ALMC_LEXER_H
#define ALMC_LEXER_H

#include <stdio.h>
#include <ctype.h>
#include <stdint.h>
#include <string.h>

#include "error.h"
#include "common.h"
#include "sbuffer.h"

#define EXT_CHARS 19
#define EXT_CHARS_IN_TOKEN_ENUM_OFFSET TOKEN_RIGHT_ANGLE + 1

#define CHARS 25
#define CHARS_IN_TOKEN_ENUM_OFFSET 0

#define KEYWORDS 33
#define KEYWORD_IN_TOKEN_ENUM_OFFSET TOKEN_IDNT + 1

#define TOKEN_TYPE_STR(type) (type < (KEYWORDS + CHARS + EXT_CHARS + 6) && type >= 0) ? tokens_str[type] : tokens_str[0]

const char chars[];
const char* keywords[];
const char* ext_chars[];
const char* tokens_str[];

typedef enum TokenType
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
	TOKEN_KEYWORD_CAST,
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
	TOKEN_EOF
} TokenType;

typedef enum NumericFormat
{
	FORMAT_DEC,
	FORMAT_BIN,
	FORMAT_OCT,
	FORMAT_HEX,
} NumericFormat;

typedef struct Token
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

typedef enum InputStreamType
{
	STREAM_FILE,
	STREAM_CHAR_PTR
} InputStreamType;

typedef struct LexerBackupData
{
	uint32_t prev_line;
	uint32_t prev_line_offset;
} LexerBackupData;

typedef struct Lexer
{
	Token* tokens;

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


Lexer* lexer_new(const char* src, InputStreamType type);
Token* token_new(TokenType type, SrcContext* context);
char* token_tostr(Token* token);

Token* lexer_get_tokens(Lexer* lex);

void scomment(Lexer* lex);
void mcomment(Lexer* lex);

char get_next_char(Lexer* lex);
void unget_curr_char(Lexer* lex);

int get_tokens_format(Lexer* lex);
Token* get_eof_token(Lexer* lex);
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

#endif //LEXER_H