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
    gen_expr(ast->stmts[0]->expr_stmt->expr);
    //print_ast(ast);
    lexer_free(lexer);
    parser_free(parser);
    ast_free(ast);
    visitor_free(visitor);

    /*char* indent = "   ";
    while (true) {
        char buffer[64]
        printf("> ");
        scanf("%s", buffer);
        Lexer* lexer = lexer_new(buffer, FROM_CHAR_PTR);
        Parser* parser = parser_new(lexer->curr_file, lex(lexer));
        frmt("%s", type_tostr_plain(parse_type(parser)));
        //char* indent = frmt("%s   ", indent);
        //printf("%s\n", indent);
        for (size_t i = 0; i < _get_frmt_buffer_ptr(); i++) {
            putc(_frmt_buffer[i], stdout);            
        }
        printf("\n");
        //printf("%s, %d\n", _frmt_buffer, strlen(_frmt_buffer));
    }*/
    /*char* path = get_root("adsas");
    printf("%s\n", path);
    while (true) {
        char a[256];
        scanf("%s", a);
        path = path_combine(path, a);
        printf("%s\n", path);
        printf("%ld\n", strlen(path));
    }*/
    /*Type* type = type_new("a");
    type = pointer_type_new(type);
    type = array_type_new(type, expr_new(EXPR_CONST, (void*)const_new(CONST_INT, "5", NULL)));
    printf("%s\n", type_tostr_plain(type));*/
    return 0;
}