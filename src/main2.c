#include <stdio.h>

#include "front-end/lexer.h"
#include "front-end/token.h"

int main(int argc, char** argv) {
    Lexer* lexer = lexer_new("asdasdasd if else case 123e+6 ;; .", FROM_CHAR_PTR);
    Token** tokens = lex(lexer);
    for (size_t i = 0; i < sbuffer_len(tokens); i++) {
        printf("%s\n", token_type_tostr(tokens[i]->type));
    }
    lexer_free(lexer);
    return 0;
}