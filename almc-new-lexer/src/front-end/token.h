#ifndef ALMC_LEXER_TOKEN_H
#define ALMC_LEXER_TOKEN_H

#include "..\utils\common.h"
#include "..\utils\context.h"

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
	TOKEN_KEYWORD_ELIF,
	TOKEN_KEYWORD_EXTERN,
	TOKEN_KEYWORD_FOR,
	TOKEN_KEYWORD_FUNC,
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
	TOKEN_KEYWORD_STRING,
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

typedef struct Token
{
	TokenType type;
	SrcContext* context;
	union
	{
		char cvalue;
		char* svalue;
		uint64_t uvalue;
	};
} Token;

Token* token_new(TokenType type, SrcContext* context);
void token_free(Token* token);

char* token_tostr(Token* token);
char* token_type_tostr(TokenType type);

#endif