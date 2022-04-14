#include "almc.h"
#include "..\test\test.h"
#include "..\src\utils\data-structures\linked-list\linked-list.h"

int back_end_test()
{
	while (1)
	{
		Lexer* lexer = lexer_new("example\\test4\\test9.almc", FROM_FILE);
		//Lexer* lexer = lexer_new("test\\test-cases\\parser-test-cases\\parser-ast-manual-tests\\testfolder\\back_end_test.almc",FROM_FILE);
		Parser* parser = parser_new(lexer->curr_file, lex(lexer));
		AstRoot* ast = parse(parser);
		Visitor* visitor = visitor_new();
		visit(ast, visitor);
		print_ast(ast);

		lexer_free(lexer);
		ast_free(ast);
		parser_free(parser);
		visitor_free(visitor);

		//AsmProgram* program = gen(ast, visitor->global);
		//print_program(program);
		//print_program_to_file(program);
		//program_free(program);
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

void expr_evaluator_test()
{
	while (1)
	{
		char buffer[50];
		char* input = gets(buffer);
		Lexer* lexer = lexer_new(input, FROM_CHAR_PTR);
		Parser* parser = parser_new(NULL, lex(lexer));
		AstRoot* ast = parse(parser);
		Visitor* visitor = visitor_new();
		visit(ast, visitor);

		Expr* expr = ast->stmts[0]->expr_stmt->expr;
		is_integral_type(retrieve_expr_type(expr)) ?
			printf("%d\n", (int)evaluate_expr_itype(expr)) : 
				printf("%f\n", (float)evaluate_expr_ftype(expr));
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
			(file_exists(options->target.src_object_path) ? "/s" : "/a"), 
				options->target.root));
		system(frmt("replace %s %s %s", options->target.binary_path,
			(file_exists(options->target.src_binary_path) ? "/s" : "/a"),
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

void data_structures_test()
{
	int* test = NULL;
	sbuffer_add(test, 2);
	/*LinkedList* list = NULL;

	llist_add(list, idnt_new("test1", NULL));
	llist_add(list, idnt_new("test2", NULL));
	llist_add(list, idnt_new("test3", NULL));
	llist_add(list, idnt_new("test4", NULL));

	Idnt* val = llist_get(Idnt*, list, 2);
	llist_free_fully(list, idnt_free);*/
}

int regtable_test()
{
	RegisterTable* table = regtable_new();

	for (int i = EAX; i <= EDX; i += 3)
		for (int j = 0; j < 3; j++)
		{
			printf("%d) %s\n", i + j, get_register_str(i + j));
			if ((i + j) % 3 == 0)
				reserve_register(table, i + j);
		}

	for (int i = XMM0; i <= XMM7; i += 1)
		printf("%d) %s\n", i, get_register_str(i)),
			reserve_register(table, i);

	unreserve_register(table, XMM6);
	int reg = get_unreserved_register(table, REGSIZE_PACKED);

	return 1;
}

void conversion_test()
{
	//Const* cnst = const_new(CONST_FLOAT, "1.3e-24", NULL);
	//gen_const_sse(cnst, NULL);
	char buf[64];
	sprintf_s(buf, sizeof buf, "%g", 1.3e-24);
	sprintf_s(buf, sizeof buf, "%g", 3.14151692);
	sprintf_s(buf, sizeof buf, "%g", 1.0);
	sprintf_s(buf, sizeof buf, "%g", 0.132123);
	sprintf_s(buf, sizeof buf, "%g", 11.0e+24);
}

void fprintf_test()
{
	char header[256];
	sprintf_s(header, sizeof header, "%s", "ERROR");

	char message[256];
	sprintf_s(message, sizeof message, "%s", "error message");

	char** spelling = NULL;
	char* line1 = (char*)malloc(sizeof(256));
	char* line2 = (char*)malloc(sizeof(256));

	SrcContext* context = src_context_new("unknown", 0, 15, 4);

	report_info_base(header, message, NULL, src_context_tostr, context);
}

int main(int argc, char** argv)
{
	//fprintf_test();
	//conversion_test();
	//regtable_test();
	//data_structures_test();
	//expr_evaluator_test();
	//run_tests(); 
	back_end_test();
	//options = parse_options(argv, argc);
	//compile();
	//assemble_and_link();
	//options_free(options);
	return 0;
}