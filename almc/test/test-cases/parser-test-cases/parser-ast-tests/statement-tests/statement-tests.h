#ifndef ALMC_GLOBAL_AST_STATEMENT_TEST
#define ALMC_GLOBAL_AST_STATEMENT_TEST

inline void test_func_decl_stmts()
{
	Lexer* lexer = lexer_new(
		"test\\test-cases\\parser-test-cases\\parser-ast-tests\\statement-tests\\declaration_stmts\\func_decl.almc", FROM_FILE);
	char* path_copy = _strdup(lexer->curr_file);
	Parser* parser = parser_new(path_copy, lex(lexer));
	AstRoot* ast = parse(parser);

	assert(sbuffer_len(ast->stmts) == 4);
	assert(ast->stmts[0]->type == STMT_FUNC_DECL);
	assert(strcmp(ast->stmts[0]->func_decl->func_name, "a_1") == 0);
	assert(!ast->stmts[0]->func_decl->func_body);
	assert(sbuffer_len(ast->stmts[0]->func_decl->func_params) == 0);
	assert(ast->stmts[0]->func_decl->func_type->mods.is_void);

	assert(ast->stmts[1]->type == STMT_FUNC_DECL);
	assert(strcmp(ast->stmts[1]->func_decl->func_name, "a_2") == 0);
	assert(!ast->stmts[1]->func_decl->func_body);
	assert(strcmp(ast->stmts[1]->func_decl->func_params[0]->type->repr, "i32") == 0);
	assert(strcmp(ast->stmts[1]->func_decl->func_params[1]->var, "b") == 0);
	assert(strcmp(ast->stmts[1]->func_decl->func_type->repr, "i32") == 0);

	assert(ast->stmts[2]->type == STMT_FUNC_DECL);
	assert(strcmp(ast->stmts[2]->func_decl->func_name, "a_3") == 0);
	assert(ast->stmts[2]->func_decl->func_spec.is_forward);
	assert(!ast->stmts[2]->func_decl->func_body);
	assert(sbuffer_len(ast->stmts[2]->func_decl->func_params) == 0);
	assert(ast->stmts[2]->func_decl->func_type->mods.is_void);

	assert(ast->stmts[3]->type == STMT_FUNC_DECL);
	assert(strcmp(ast->stmts[3]->func_decl->func_name, "a_4") == 0);
	assert(ast->stmts[3]->func_decl->func_spec.is_intrinsic);
	assert(!ast->stmts[3]->func_decl->func_body);
	assert(sbuffer_len(ast->stmts[3]->func_decl->func_params) == 0);
	assert(ast->stmts[3]->func_decl->func_type->mods.is_void);

	free(path_copy);
	lexer_free(lexer);
	parser_free(parser);
	clear_imported_modules();
	ast_free(ast);
}

inline void test_type_decl_stmts()
{
	Lexer* lexer = lexer_new(
		"test\\test-cases\\parser-test-cases\\parser-ast-tests\\statement-tests\\declaration_stmts\\type_decl.almc", FROM_FILE);
	char* path_copy = _strdup(lexer->curr_file);
	Parser* parser = parser_new(path_copy, lex(lexer));
	AstRoot* ast = parse(parser);

	assert(sbuffer_len(ast->stmts) == 11);
	assert(ast->stmts[0]->type_decl->type == TYPE_DECL_STRUCT);
	assert(strcmp(ast->stmts[0]->type_decl->struct_decl->struct_name, "a") == 0);
	assert(!ast->stmts[0]->type_decl->struct_decl->struct_mmbrs);

	assert(ast->stmts[1]->type_decl->type == TYPE_DECL_STRUCT);
	assert(strcmp(ast->stmts[1]->type_decl->struct_decl->struct_name, "") == 0);
	assert(strcmp(ast->stmts[1]->type_decl->struct_decl->struct_mmbrs[0]->var, "noname") == 0);

	assert(ast->stmts[2]->type_decl->type == TYPE_DECL_STRUCT);
	assert(strcmp(ast->stmts[2]->type_decl->struct_decl->struct_name, "a") == 0);
	assert(strcmp(ast->stmts[2]->type_decl->struct_decl->struct_mmbrs[0]->var, "a_m") == 0);
	assert(strcmp(ast->stmts[2]->type_decl->struct_decl->struct_mmbrs[1]->type->repr, "i16") == 0);

	assert(ast->stmts[3]->type_decl->type == TYPE_DECL_ENUM);
	assert(strcmp(ast->stmts[3]->type_decl->enum_decl->enum_name, "a_e") == 0);
	assert(!ast->stmts[3]->type_decl->enum_decl->enum_idnts);

	assert(ast->stmts[4]->type_decl->type == TYPE_DECL_ENUM);
	assert(strcmp(ast->stmts[4]->type_decl->enum_decl->enum_name, "") == 0);
	assert(ast->stmts[4]->type_decl->enum_decl->enum_idnt_values[0]->cnst->ivalue == 255);

	assert(ast->stmts[5]->type_decl->type == TYPE_DECL_ENUM);
	assert(strcmp(ast->stmts[5]->type_decl->enum_decl->enum_name, "a") == 0);
	assert(ast->stmts[5]->type_decl->enum_decl->enum_idnt_values[0]->cnst->ivalue == 0);
	assert(ast->stmts[5]->type_decl->enum_decl->enum_idnt_values[1]->cnst->ivalue == 1);
	assert(ast->stmts[5]->type_decl->enum_decl->enum_idnt_values[2]->cnst->ivalue == 2);
	assert(strcmp(ast->stmts[5]->type_decl->enum_decl->enum_idnts[1]->svalue, "s") == 0);

	assert(ast->stmts[6]->type_decl->type == TYPE_DECL_ENUM);
	assert(strcmp(ast->stmts[6]->type_decl->enum_decl->enum_name, "b") == 0);
	assert(ast->stmts[6]->type_decl->enum_decl->enum_idnt_values[0]->cnst->ivalue == 0xf);
	assert(ast->stmts[6]->type_decl->enum_decl->enum_idnt_values[1]->cnst->ivalue == (0xf + 1));
	assert(ast->stmts[6]->type_decl->enum_decl->enum_idnt_values[2]->cnst->ivalue == 1);
	assert(ast->stmts[6]->type_decl->enum_decl->enum_idnt_values[3]->cnst->ivalue == 2);
	assert(strcmp(ast->stmts[6]->type_decl->enum_decl->enum_idnts[3]->svalue, "g") == 0);

	assert(ast->stmts[7]->type_decl->type == TYPE_DECL_ENUM);
	assert(strcmp(ast->stmts[7]->type_decl->enum_decl->enum_name, "c") == 0);
	assert(ast->stmts[7]->type_decl->enum_decl->enum_idnt_values[0]->type == EXPR_BINARY_EXPR);
	assert(ast->stmts[7]->type_decl->enum_decl->enum_idnt_values[1]->binary_expr->rexpr->cnst->ivalue == 1);
	assert(ast->stmts[7]->type_decl->enum_decl->enum_idnt_values[2]->cnst->ivalue == 0xf);
	assert(ast->stmts[7]->type_decl->enum_decl->enum_idnt_values[3]->cnst->ivalue == (0xf + 1));
	assert(strcmp(ast->stmts[7]->type_decl->enum_decl->enum_idnts[2]->svalue, "t") == 0);

	assert(ast->stmts[8]->type_decl->type == TYPE_DECL_UNION);
	assert(strcmp(ast->stmts[8]->type_decl->union_decl->union_name, "a") == 0);
	assert(!ast->stmts[8]->type_decl->union_decl->union_mmbrs);

	assert(ast->stmts[9]->type_decl->type == TYPE_DECL_UNION);
	assert(strcmp(ast->stmts[9]->type_decl->union_decl->union_name, "") == 0);
	assert(strcmp(ast->stmts[9]->type_decl->union_decl->union_mmbrs[0]->type->repr, "i32") == 0);

	assert(ast->stmts[10]->type_decl->type == TYPE_DECL_UNION);
	assert(strcmp(ast->stmts[10]->type_decl->union_decl->union_name, "b_u") == 0);
	assert(strcmp(ast->stmts[10]->type_decl->union_decl->union_mmbrs[0]->type->repr, "i32") == 0);
	assert(strcmp(ast->stmts[10]->type_decl->union_decl->union_mmbrs[1]->var, "c") == 0);

	free(path_copy);
	lexer_free(lexer);
	parser_free(parser);
	clear_imported_modules();
	ast_free(ast);
}

inline void test_var_decl_stmts()
{
	Lexer* lexer = lexer_new(
		"test\\test-cases\\parser-test-cases\\parser-ast-tests\\statement-tests\\declaration_stmts\\var_decl.almc", FROM_FILE);
	char* path_copy = _strdup(lexer->curr_file);
	Parser* parser = parser_new(path_copy, lex(lexer));
	AstRoot* ast = parse(parser);

	assert(sbuffer_len(ast->stmts) == 2);
	assert(ast->stmts[0]->type == STMT_VAR_DECL);
	assert(strcmp(ast->stmts[0]->var_decl->type_var->var, "a") == 0);
	assert(strcmp(ast->stmts[0]->var_decl->type_var->type->repr, "i32") == 0);
	assert(!ast->stmts[0]->var_decl->var_init);

	assert(ast->stmts[1]->type == STMT_VAR_DECL);
	assert(strcmp(ast->stmts[1]->var_decl->type_var->var, "b") == 0);
	assert(strcmp(ast->stmts[1]->var_decl->type_var->type->repr, "i32") == 0);
	assert(ast->stmts[1]->var_decl->var_init->cnst->ivalue == 2);

	free(path_copy);
	lexer_free(lexer);
	parser_free(parser);
	clear_imported_modules();
	ast_free(ast);
}

inline void test_loop_stmts()
{

}

inline void test_jump_stmts()
{

}

inline void ast_stmt_tests()
{
	test_func_decl_stmts();
	test_type_decl_stmts();
	test_var_decl_stmts();
	test_loop_stmts();
}

#endif