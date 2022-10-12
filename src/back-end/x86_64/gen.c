#include "gen.h"

regid regtable[regs_count] = {0};
global_gen_data global_gen_info = {0};
// todo: finish alloc functions

char* alloc_float_definition() {
    size_t count = global_gen_info.count_of.floats;
    char* definition = cnew(char, 64);
    sprintf(definition, "_FCONST%ld", count);
    global_gen_info.count_of.floats++;
    sbuffer_add(global_gen_info.allocated_definitions, definition);
    return definition;
}

char* alloc_string_definition() {
    size_t count = global_gen_info.count_of.floats;
    char* definition = cnew(char, 64);
    sprintf(definition, "_STRCONST%ld", count);
    global_gen_info.count_of.floats++;
    sbuffer_add(global_gen_info.allocated_definitions, definition);
    return definition;
}

gen_data gen_expr_stmt(ExprStmt* expr_stmt) {
    return gen_expr(expr_stmt->expr);
}

gen_data gen_stmt(Stmt* stmt) {
    switch (stmt->kind) {
        case STMT_EXPR:
            return gen_expr_stmt(stmt->expr_stmt);
        case STMT_FUNC_DECL:
            break;
        case STMT_TYPE_DECL:
            break;
        default:
            report_error(frmt("Unknown stmt kind met"
                " in function: %s", __FUNCTION__), NULL);
    }
    // to avoid warning
    gen_data data = {0};
    return data;
}

void global_gen_info_init(Assembly* assembly) {
    memset(&global_gen_info, 0, sizeof(global_gen_data));
    global_gen_info.curr_asm = assembly;
    global_gen_info.curr_bss_seg = assembly->bss;
    global_gen_info.curr_data_seg = assembly->data;
    global_gen_info.curr_text_seg = assembly->text;
    global_gen_info.allocated_definitions = NULL;
}

void global_gen_data_free() {
    for (size_t i = 0; i < sbuffer_len(global_gen_info.allocated_definitions); i++) {
        free(global_gen_info.allocated_definitions[i]);
    }
    sbuffer_free(global_gen_info.allocated_definitions);
}

Assembly* gen(AstRoot* ast) {
    Assembly* assembly = asm_new("_start");
    global_gen_info_init(assembly);
    for (size_t i = 0; i < sbuffer_len(ast->stmts); i++) {
        gen_data data = gen_stmt(ast->stmts[i]);
        for (size_t i = 0; i < sbuffer_len(data.text_instrs); i++) {
            sbuffer_add(assembly->text->instructions, data.text_instrs[i]);
        }
        for (size_t i = 0; i < sbuffer_len(data.data_instrs); i++) {
            sbuffer_add(assembly->data->instructions, data.data_instrs[i]);
        }
    }
    return assembly;
}

gen_data gen_binary_expr(BinaryExpr* binary_expr) {
    ;
}

gen_data gen_expr(Expr* expr) {
    if (is_const_expr(expr) && expr->kind != EXPR_CONST) {
        return gen_const_expr(expr);
    }
    switch (expr->kind) {
        case EXPR_CONST:
            return gen_const(expr->cnst);
        case EXPR_STRING:
            return gen_str_const(expr->str);
        case EXPR_BINARY_EXPR:
            return gen_binary_expr(expr->binary_expr);
        default:
            report_error(frmt("Unknown expression kind met"
                " in function: %s", __FUNCTION__), NULL);
    }
}

gen_data gen_const_expr(Expr* expr) {
    char buf[64];
    Const const_expr;
    Type* expr_type = retrieve_expr_type(expr);
    const_expr.type = expr_type;
    if (is_integral_type(expr_type)) {
        int64_t eval = evaluate_expr_itype(expr);
        if (is_unsigned_type(expr_type)) {
            sprintf(buf, "%lu", eval);
            const_expr.kind = CONST_UINT;
        } else {
            sprintf(buf, "%ld", eval);
            const_expr.kind = CONST_INT;
        }
        const_expr.svalue = buf;
        const_expr.ivalue = eval;
    }
    else {
        double eval = evaluate_expr_ftype(expr);
        sprintf(buf, "%f", eval);
        char* heapval = cnew(char, sizeof buf);
        sprintf(heapval, "%s", buf);
        const_expr.fvalue = eval;
        const_expr.kind = CONST_FLOAT;
        const_expr.svalue = heapval;
        sbuffer_add(global_gen_info.allocated_definitions, heapval);
    }
    return gen_const(&const_expr);
}

char** gen_str_const_values(Str* str_const_expr) {
    char* value = NULL;
	char** values = NULL;
    bool is_value_empty = false;

    //todo: finish this function 

	/*
		Logic below splits control characters and normal text
		to different item, all these will be appended to string definition
		in assembly source
		e.g:
		[INPUT]			"test\n\r"
		[OUTPUT]		"test" "0ah" "0dh"
	*/

    /*sbuffer_add(value, "\"");
    for (size_t i = 0; str_const_expr->svalue[i]; i++) {
		if (!isescape(str_const_expr->svalue[i])) {
            sbuffer_add(value, str_const_expr->svalue[i]);
		}
		else {
            sbuffer_add(value, "\"");
			value = frmt("%s\"", value);
			if (!is_value_empty) {
				sbuffer_add(values, value);
            }
			is_value_empty = true;
			value = frmt("%02xh", (int)str->svalue[i]);
			sbuffer_add(values, value);
			value = frmt("\"");
		}
		free(value_for_freeing);
	}
	if (!is_value_empty) {
		sbuffer_add(values, frmt("%s\"", value));
        free(value);
    }
	sbuffer_add(values, frmt("00h"));*/
}

gen_data gen_str_const(Str* str_const_expr) {
    gen_data data = {0};
    regid storage = reserve_any(REGQ);
    char* definition = alloc_string_definition();
    char** values = NULL;
    sbuffer_add(values, str_const_expr->svalue);
    DataInstruction* instr1 = data_instr(definition, values, SPEC_BYTE);
    TextInstruction* instr2 = instr_bin(MOV);
    instr2->left = instrarg(reg_tostr(storage));
    instr2->right = instrarg(definition);
    sbuffer_add(data.data_instrs, instr1);
    sbuffer_add(data.text_instrs, instr2);
    data.reg = storage;
    data.is_in_reg = true;
    return data;
}

gen_data gen_const(Const* const_expr) {
    regid storage = -1;
    gen_data data = {0};
    if (const_expr->kind == CONST_FLOAT) {
        storage = reserve_any(REGV);
        char** values = NULL;
        sbuffer_add(values, const_expr->svalue);
        InstructionArgumentSize size = SPEC_DWORD;
        if (is_f64_type(const_expr->type)) {
            size = SPEC_QWORD;
        }
        char* definition = alloc_float_definition();
        DataInstruction* instr1 = data_instr(definition, values, size);
        TextInstruction* instr2 = instr_bin(MOVSS);
        instr2->left = instrarg(reg_tostr(storage));
        instr2->right = instrarg(definition);
        instr2->right->dereference = true;
        sbuffer_add(data.data_instrs, instr1);
        sbuffer_add(data.text_instrs, instr2);
    } 
    else {
        char buf[64];
        storage = reserve_any(const_expr->type->size*8);
        if (is_unsigned_type(const_expr->type)) {
            sprintf(buf, "%lu", const_expr->uvalue);
        } else {
            sprintf(buf, "%ld", const_expr->ivalue);
        }
        char* heapval = cnew(char, sizeof buf);
        sbuffer_add(global_gen_info.allocated_definitions, heapval);
        sprintf(heapval, "%s", buf);
        TextInstruction* instr1 = instr_bin(MOV);
        instr1->left = instrarg(reg_tostr(storage));
        instr1->right = instrarg(heapval);
        sbuffer_add(data.text_instrs, instr1);
    }
    data.reg = storage;
    data.is_in_reg = true;
    return data;
}