#include "sbuffer.h"

#define lexer_init(x) (x = malloc(sizeof(Lexer)))
#define token_init(x) (t = malloc(sizeof(Token)))

/*static char* keywords[6] = {
	"sizeof",
	"typeof",
	"for",
	"if",
	"void",
	"int"
};*/

typedef enum
{
	RPAREN_T,
	LPAREN_T
} TokenType;

typedef struct
{
	char* file;
	size_t sym;
	size_t line;
} SrcContext;

typedef struct
{
	char* value;
	TokenType type;
	SrcContext context;
} Token;

typedef struct
{
	Token* tokens;
	size_t curr_index;
} Lexer;

SrcContext* context_create(char* file, size_t sym, size_t line);
Token* token_create(char* value, TokenType type, SrcContext context);

Token* lexer_get_tokens(const char* stream, char is_file);