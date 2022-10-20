#include <stdio.h>

#include "almc.h"

int main(int argc, char** argv) {
    char filepath[] = "/home/almeswe/Projects/almc/sandbox/parser_test.almc";
    Lexer* lexer = lexer_new(filepath, FROM_FILE);
    Token** tokens = lex(lexer); 
    Parser* parser = parser_new(lexer->curr_file, tokens);
    AstRoot* ast = parse(parser);
    Visitor* visitor = visitor_new();
    visit(ast, visitor);
    Assembly* assembly = gen(ast);
    asm_to_stream(assembly, stdout);
    lexer_free(lexer);
    parser_free(parser);
    ast_free(ast);
    visitor_free(visitor);
    asm_free(assembly);
    global_gen_data_free();
    return 0;
}