#include "almc.h"
#include "..\test\test.h"

int back_end_test()
{
	while (1)
	{
		Lexer* lexer = lexer_new("example\\linked_list\\list_test.almc", FROM_FILE);
		//Lexer* lexer = lexer_new("test\\test-cases\\parser-test-cases\\parser-ast-manual-tests\\testfolder\\back_end_test.almc",FROM_FILE);
		Parser* parser = parser_new(lexer->curr_file, lex(lexer));
		AstRoot* ast = parse(parser);
		Visitor* visitor = visitor_new();
		visit(ast, visitor);

		AsmProgram* program = gen(ast, visitor->global);
		print_program(program);
		//print_program_to_file(program);
		program_free(program);
		char c = getchar();
		system("cls");
	}
}

void type2_test()
{
	while (1)
	{
		char buffer[30];
		char* input = gets(buffer);
		system("cls");

		Lexer* lexer = lexer_new(
			input, FROM_CHAR_PTR);
		Parser* parser = parser_new(NULL, lex(lexer));
		Type* type;
		print_type(type = parse_type(parser), "");
		type_free(type);
	}
}

void cast_type_test()
{
	Lexer* lexer = lexer_new(
		"sizeof(i32)", FROM_CHAR_PTR);
	Parser* parser = parser_new(NULL, lex(lexer));
	Expr* expr = parse_expr(parser);
	return;
}

void assemble_and_link()
{
	system(frmt("%s /c /Zd /coff %s", options->compiler.ml_path,
		options->target.asm_path));
	if (file_exists(options->target.object_path))
	{
		system(frmt("%s /SUBSYSTEM:CONSOLE %s", options->compiler.link_path,
			options->target.object_path));
		//replace .obj .exe file to source folder 
		system(frmt("replace %s %s %s", options->target.object_path,
			file_exists(options->target.src_object_path) ? "/s" : "/a", 
			options->target.root));
		system(frmt("replace %s %s %s", options->target.binary_path,
			file_exists(options->target.src_binary_path) ? "/s" : "/a",
				options->target.root));
		
		system(frmt("del %s", options->target.binary_path));
		system(frmt("del %s", options->target.object_path));
	}	
}

void compile()
{
	Lexer* lexer = lexer_new(
		options->target.target_path, FROM_FILE);
	Parser* parser = parser_new(options->target.target_path,
		lex(lexer));
	AstRoot* ast = parse(parser);
	Visitor* visitor = visitor_new();
	visit(ast, visitor);
	AsmProgram* program = gen(ast, visitor->global);
	print_program_to_file(program);
	lexer_free(lexer);
	ast_free(ast);
	parser_free(parser);
	visitor_free(visitor);
	program_free(program);
}

int main(int argc, char** argv)
{
	options = parse_options(argv, argc);
	compile();
	assemble_and_link();
	options_free(options);
	//back_end_test();
	//run_tests();
	return 0;
}