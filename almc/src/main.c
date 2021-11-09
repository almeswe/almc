#include "..\test\test.h"

#include "back-end/x86 asm/stack-frame.h"
#include "back-end/x86 asm/gen.h"

int test()
{
	while (1)
	{
		Lexer* lexer = lexer_new(
			"test\\test-cases\\parser-test-cases\\parser-ast-manual-tests\\testfolder\\back_end_test.almc", FROM_FILE);
		char* path_copy = _strdup(lexer->curr_file);
		Parser* parser = parser_new(path_copy, lex(lexer));
		AstRoot* ast = parse(parser);
		
		for (int i = 0; i < sbuffer_len(ast->stmts); i++)
			gen_stmt(ast->stmts[i], NULL);
		//print_expr(expr, "");
		//init_reserved_registers();
		char c = getchar();
		system("cls");
	}
}

int type_test()
{
	while (1)
	{
		Type* type = cnew_s(Type, type, 1);
		type->mods.is_predefined = 1;
		type->mods.is_ptr = 2;
		type->repr = "i32";

		Type* type1 = cnew_s(Type, type, 1);
		type1->mods.is_ptr = 1;
		type1->repr = "StructType";

		TypeVar** type_vars = NULL;
		sbuffer_add(type_vars, type_var_new(type, "mmbr"));

		StructDecl* struct_decl = struct_decl_new(
			type_vars, "StructType");

		VarDecl* var_decl = var_decl_new(type_var_new(
			type1, "a"), NULL);

		char buffer[50];
		char* input = gets(buffer);
		Lexer* lexer = lexer_new(
			input, FROM_CHAR_PTR);
		Parser* parser = parser_new(NULL, lex(lexer));
		Expr* expr = parse_expr(parser);
		
		Visitor* visitor = visitor_new();
		add_variable(var_decl, visitor->global);
		add_struct(struct_decl, visitor->global);

		get_expr_type(expr, visitor->global);

		print_expr(expr, "");

		lexer_free(lexer);
		parser_free(parser);
		visitor_free(visitor);
		expr_free(expr);
		var_decl_free(var_decl);
		clear_imported_modules();
	}
}

int main(int argc, char** argv)
{
	//test();
	type_test();
	run_tests();
	return 0;
}