#include "token.h"

const char* tokens_str[] = {
	"TOKEN_PLUS",
	"TOKEN_DASH",
	"TOKEN_ASTERISK",
	"TOKEN_SLASH",
	"TOKEN_MODULUS",
	"TOKEN_BAR",
	"TOKEN_TILDE",
	"TOKEN_CARET",
	"TOKEN_S_QUOTE",
	"TOKEN_D_QUOTE",
	"TOKEN_EXCL_MARK",
	"TOKEN_COMMA",
	"TOKEN_DOT",
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

	"TOKEN_BW_OR_ASSIGN",
	"TOKEN_BW_AND_ASSIGN",
	"TOKEN_BW_XOR_ASSIGN",
	"TOKEN_BW_NOT_ASSIGN",

	"TOKEN_LG_OR",
	"TOKEN_LG_EQ",
	"TOKEN_LG_NEQ",
	"TOKEN_LG_AND",

	"TOKEN_LSHIFT",
	"TOKEN_RSHIFT",
	"TOKEN_LESS_EQ_THAN",
	"TOKEN_GREATER_EQ_THAN",
	"TOKEN_ARROW",
	"TOKEN_DOUBLE_DOT",
	"TOKEN_TRIPLE_DOT",
	"TOKEN_NAV_CURR_DIR",
	"TOKEN_NAV_PREV_DIR",

	"TOKEN_STRING",
	"TOKEN_CHARACTER",
	"TOKEN_INT_CONST",
	"TOKEN_UINT_CONST",
	"TOKEN_FLOAT_CONST",
	"TOKEN_IDNT",

	"TOKEN_KEYWORD_BREAK",
	"TOKEN_KEYWORD_CASE",
	"TOKEN_KEYWORD_CAST",
	"TOKEN_KEYWORD_CONTINUE",
	"TOKEN_KEYWORD_DEFAULT",
	"TOKEN_KEYWORD_ENUM",
	"TOKEN_KEYWORD_ELIF",
	"TOKEN_KEYWORD_ENTRY",
	"TOKEN_KEYWORD_EXTERN",
	"TOKEN_KEYWORD_FOR",
	"TOKEN_KEYWORD_FUNC",
	"TOKEN_KEYWORD_FROM",
	"TOKEN_KEYWORD_FLOAT32",
	"TOKEN_KEYWORD_FLOAT64",
	"TOKEN_KEYWORD_GOTO",
	"TOKEN_KEYWORD_IF",
	"TOKEN_KEYWORD_INT8",
	"TOKEN_KEYWORD_INT16",
	"TOKEN_KEYWORD_INT32",
	"TOKEN_KEYWORD_INT64",
	"TOKEN_KEYWORD_IMPORT",
	"TOKEN_KEYWORD_RETURN",
	"TOKEN_KEYWORD_STRUCT",
	"TOKEN_KEYWORD_SWITCH",
	"TOKEN_KEYWORD_SIZEOF",
	"TOKEN_KEYWORD_TYPEDEF",
	"TOKEN_KEYWORD_UNION",
	"TOKEN_KEYWORD_UINT8",
	"TOKEN_KEYWORD_UINT16",
	"TOKEN_KEYWORD_UINT32",
	"TOKEN_KEYWORD_UINT64",
	"TOKEN_KEYWORD_LET",
	"TOKEN_KEYWORD_LABEL",
	"TOKEN_KEYWORD_LENGTHOF",
	"TOKEN_KEYWORD_VAR",
	"TOKEN_KEYWORD_VOID",
	"TOKEN_KEYWORD_WHILE",
	"TOKEN_KEYWORD_DO",
	"TOKEN_KEYWORD_ELSE",
	"TOKEN_EOF",
};

Token* token_new(TokenKind type, SrcContext* context) {
	Token* token = new(Token);
	token->lexeme = 0; // union's initialization here
	token->type = type;
	token->attrs.context = context;
	return token;
}

void token_free(Token* token) {
	if (token) {
		//todo: if its not keyword, than lexeme can be cleared
		src_context_free(token->attrs.context);
		free(token);
	}
}

const char* token_tostr(const Token* token) {
	char* tok_str = frmt("%s: %s", token_type_tostr(token->type), token->lexeme);
	return frmt("%s %s", tok_str, src_context_tostr(token->attrs.context));
}

const char* token_type_tostr(TokenKind type) {
	return (type >= 0 && type <= TOKEN_EOF) ?
		tokens_str[type] : tokens_str[0];
}