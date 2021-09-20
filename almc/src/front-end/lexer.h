#ifndef ALMC_LEXER_H
#define ALMC_LEXER_H

#include <stdio.h>
#include <ctype.h>
#include <stdint.h>
#include <string.h>

#include "..\error.h"
#include "..\utils\common.h"
#include "..\utils\sbuffer.h"

#define EXT_CHARS 22
#define EXT_CHARS_IN_TOKEN_ENUM_OFFSET TOKEN_RIGHT_ANGLE + 1

#define CHARS 26
#define CHARS_IN_TOKEN_ENUM_OFFSET 0

#define KEYWORDS 37
#define KEYWORD_IN_TOKEN_ENUM_OFFSET TOKEN_IDNT + 1

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
	TOKEN_DOT,
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

	TOKEN_BW_OR_ASSIGN,
	TOKEN_BW_AND_ASSIGN,
	TOKEN_BW_XOR_ASSIGN,
	TOKEN_BW_NOT_ASSIGN,

	TOKEN_LG_OR,
	TOKEN_LG_EQ,
	TOKEN_LG_NEQ,
	TOKEN_LG_AND,
	
	TOKEN_LSHIFT,
	TOKEN_RSHIFT,
	TOKEN_LESS_EQ_THAN,
	TOKEN_GREATER_EQ_THAN,
	TOKEN_INC,
	TOKEN_DEC,	
	TOKEN_ARROW,

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
	TOKEN_KEYWORD_ENUM,
	TOKEN_KEYWORD_EXTERN,
	TOKEN_KEYWORD_FOR,
	TOKEN_KEYWORD_FALSE,
	TOKEN_KEYWORD_FLOAT32,
	TOKEN_KEYWORD_FLOAT64,
	TOKEN_KEYWORD_GOTO,
	TOKEN_KEYWORD_IF,
	TOKEN_KEYWORD_INT8,
	TOKEN_KEYWORD_INT16,
	TOKEN_KEYWORD_INT32,
	TOKEN_KEYWORD_INT64,
	TOKEN_KEYWORD_REGISTER,
	TOKEN_KEYWORD_RETURN,
	TOKEN_KEYWORD_STATIC,
	TOKEN_KEYWORD_STRUCT,
	TOKEN_KEYWORD_SWITCH,
	TOKEN_KEYWORD_SIZEOF,
	TOKEN_KEYWORD_TRUE,
	TOKEN_KEYWORD_TYPEOF,
	TOKEN_KEYWORD_UNION,
	TOKEN_KEYWORD_UINT8,
	TOKEN_KEYWORD_UINT16,
	TOKEN_KEYWORD_UINT32,
	TOKEN_KEYWORD_UINT64,
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
void lexer_free(Lexer* lexer);
void token_free(Token* token);
char* token_tostr(Token* token);
char* token_type_tostr(TokenType type);

Token* lex(Lexer* lexer);

Token* get_eof_token(Lexer* lexer);
Token* get_num_token(Lexer* lexer);
Token* get_bin_num_token(Lexer* lexer);
Token* get_hex_num_token(Lexer* lexer);
Token* get_oct_num_token(Lexer* lexer);
Token* get_dec_num_token(Lexer* lexer);
Token* get_dec_fnum_token(Lexer* lexer, uint64_t base_inum, uint32_t size);

Token* get_idnt_token(Lexer* lexer);
Token* get_char_token(Lexer* lexer);
Token* get_string_token(Lexer* lexer);
Token* get_keychar_token(Lexer* lexer, int order);
Token* get_keyword_token(Lexer* lexer, int order);

#endif //LEXER_H