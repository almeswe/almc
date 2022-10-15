#include "gen.h"

regid regtable[regs_count] = {0};
global_gen_data glob_info = {0};

#define puttext(instr)   sbuffer_add(data.text_instrs, instr) 
#define putdata(instr)   sbuffer_add(data.data_instrs, instr)
#define putalloc(data)   sbuffer_add(glob_info.allocated_defs, data)

#define is_primitive_or_pointer(type) (is_primitive_type(type) || is_pointer_like_type(type))

char* alloc_float_def() {
    char* def = cnew(char, 64);
    sprintf(def, "__real@%ld",
        glob_info.count_of.floats++);
    return putalloc(def), def;
 }

char* alloc_label_def() {
    char* def = cnew(char, 64);
    sprintf(def, "_L@%ld", 
        glob_info.count_of.labels++);
    return putalloc(def), def;
}

char* alloc_str_def() {
    char* def = cnew(char, 64);
    sprintf(def, "__str@%ld", 
        glob_info.count_of.floats++);
    return putalloc(def), def;
}

char* alloc_func_def(const char* base) {
    char* def = cnew(char, strlen(base)+2);
    sprintf(def, "_%s", base);
    return putalloc(def), def;
}

char* alloc_var_def(const char* base) {
    char* def = cnew(char, strlen(base)+3);
    sprintf(def, "_%s$", base);
    return putalloc(def), def;
}

void global_gen_info_init(Assembly* assembly) {
    memset(&glob_info, 0, sizeof(global_gen_data));
    glob_info.curr_asm = assembly;
    glob_info.curr_bss_seg = assembly->bss;
    glob_info.curr_data_seg = assembly->data;
    glob_info.curr_text_seg = assembly->text;
    glob_info.allocated_defs = NULL;
}

void global_gen_data_free() {
    for (size_t i = 0; i < sbuffer_len(glob_info.allocated_defs); i++) {
        free(glob_info.allocated_defs[i]);
    }
    sbuffer_free(glob_info.allocated_defs);
}

void merge_gen_data(gen_data* data_out, gen_data* data_in) {
    data_out->reg = data_in->reg;
    //data_out->is_in_reg = data_in->is_in_reg;
    for (size_t i = 0; i < sbuffer_len(data_in->text_instrs); i++) {
        sbuffer_add(data_out->text_instrs, data_in->text_instrs[i]);
    }
    for (size_t i = 0; i < sbuffer_len(data_in->data_instrs); i++) {
        sbuffer_add(data_out->data_instrs, data_in->data_instrs[i]);
    }
}

regid reserve_from_type(Type* type) {
    if (is_primitive_or_pointer(type)) {
        if (is_real_type(type)) {
            return reserve_any(REGV);
        }
        return reserve_any(type->size*CHAR_BIT);
    }
    report_error(frmt("Cannot reserve register for this type %s"
        " in function: %s", type_tostr_plain(type), __FUNCTION__), NULL);
}

TextInstruction* mov_instr(Type* type) {
    if (is_primitive_type(type)) {
        if (is_integral_type(type)) {
            return instr_bin(MOV);
        }
        if (is_f64_type(type)) {
            return instr_bin(MOVSD);
        }
        return instr_bin(MOVSS);
    } else {
        if (is_pointer_like_type(type)) {
            return instr_bin(MOV);
        }
        report_error(frmt("Cannot get mov instruction"
            " in function: %s", __FUNCTION__), NULL);
        return NULL;
    }
}

InstructionArgumentSize get_size_spec(Type* type) {
    if (is_primitive_or_pointer(type)) {
        switch (type->size) {
            case 1: return SPEC_BYTE;
            case 2: return SPEC_WORD;
            case 4: return SPEC_DWORD;
            case 8: return SPEC_QWORD;
        }
    }
    report_error(frmt("Cannot determine size of type %s, "
        "in %s", type_tostr_plain(type), __FUNCTION__), NULL);
}

Assembly* gen(AstRoot* ast) {
    Assembly* assembly = asm_new();
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

gen_data gen_expr(Expr* expr) {
    /* 
        Function which generates expression.
        Returns structure which represents the result of generation.
    */
    if (is_const_expr(expr) && expr->kind != EXPR_CONST) {
        return gen_const_expr(expr);
    }
    switch (expr->kind) {
        case EXPR_IDNT:
            return gen_idnt(expr->idnt);
        case EXPR_CONST:
            return gen_const(expr->cnst);
        //case EXPR_STRING:
        //    return gen_str_const(expr->str);
        case EXPR_UNARY_EXPR:
            return gen_unary_expr(expr->unary_expr);
        //case EXPR_BINARY_EXPR:
        //    return gen_binary_expr(expr->binary_expr);
        default:
            report_error(frmt("Unknown expression kind met"
                " in function: %s", __FUNCTION__), NULL);
    }
}

gen_data gen_idnt(Idnt* idnt) {
    /* 
        Function which generates identifier, and returns structure which
        represents the result of generation.
    */
    gen_data data = {0};
    if (is_primitive_or_pointer(idnt->type)) {
        // in case of primitive type or pointer-like type
        // storage of this identifier will be in register

        // retrieveing proper mov instruction for type of identifier (mov, movss or movsd)
        TextInstruction* movinstr = mov_instr(idnt->type);
        // allocating storage where (in case of primitive or pointer type) value of identifier will be stored
        regid storage = (data.reg = reserve_from_type(idnt->type));
        StackFrameEntity* entity = glob_frame_get(idnt->svalue);
        if (entity == NULL) {
            //todo: logic for case with global variable
            report_error(frmt("Cannot find stack frame entity with name %s, "
                "in %s", idnt->svalue, __FUNCTION__), NULL);
        }
        movinstr->left = instr_arg_reg(storage);
        movinstr->right = instr_arg_reg(entity->in.reg);
        movinstr->right->offset = entity->in.offset;
        // in case of attempt to mov from one xmm register to another,
        // there is no need for dereferencing or explicit type specification
        if (!is_simd_reg(entity->in.reg)) {
            movinstr->right->dereference = true;
            movinstr->right->spec = get_size_spec(idnt->type);
        }
        data._test_reg = RBP;
        data._test_offset = entity->in.offset;
        puttext(movinstr);
    }
    else {
        report_error("unimplemeted yet.", NULL);
    }
    return data;
}

gen_data gen_const(Const* const_expr) {
    /* 
        Function which generates constant, and returns structure which
        represents the result of generation.
    */
    gen_data data = {0};
    // allocating storage for constant value
    regid storage = (data.reg = reserve_from_type(const_expr->type));
    if (const_expr->kind == CONST_FLOAT) {
        // in case when constant has real4 or real8 type
        // it means that it must be allocated in .data section
        // and assigned to one of xmm0-xmm15 registers
        char** values = NULL;
        sbuffer_add(values, const_expr->svalue);
        InstructionArgumentSize size = SPEC_DWORD;
        if (is_f64_type(const_expr->type)) {
            size = SPEC_QWORD;
        }
        // allocating definition (alternative name) which will be
        // used to name data instruction (in data section) 
        // assigned to actual value of this constant
        char* definition = alloc_float_def();
        DataInstruction* realinstr = data_instr(
            definition, values, size, DATA_FLOAT_CONST);
        // get proper mov instruction (movss or movsd)
        TextInstruction* movinstr = mov_instr(const_expr->type);
        movinstr->left = instr_arg_reg(storage);
        movinstr->right = instr_arg(definition);
        movinstr->right->dereference = true;
        putdata(realinstr), puttext(movinstr);
    } 
    else {
        char buf[64];
        // convertion of const value to string value
        if (is_unsigned_type(const_expr->type)) {
            sprintf(buf, "%lu", const_expr->uvalue);
        } else {
            sprintf(buf, "%ld", const_expr->ivalue);
        }
        char* heapval = cnew(char, sizeof buf);
        sprintf(heapval, "%s", buf);
        // adding heapval to allocated_definitions array
        // this is probably bad design problem, but at the
        // end all allocated_definions will be freed, so i 
        // don't need to worry about this one 
        putalloc(heapval);
        TextInstruction* movinstr = instr_bin(MOV);
        movinstr->left = instr_arg_reg(storage);
        movinstr->right = instr_arg(heapval);
        puttext(movinstr);
    }
    return data;
}

gen_data gen_str_const(Str* str_const_expr) {
    gen_data data = {0};
    regid storage = (data.reg = reserve_any(REGQ));
    char* definition = alloc_str_def();
    char** values = NULL;
    sbuffer_add(values, str_const_expr->svalue);
    DataInstruction* strinstr = data_instr(
        definition, values, SPEC_BYTE, DATA_STRING_CONST);
    TextInstruction* movinstr = instr_bin(MOV);
    movinstr->left = instr_arg_reg(storage);
    movinstr->right = instr_arg(definition);
    putdata(strinstr), puttext(movinstr);
    return data;
}

gen_data gen_const_expr(Expr* expr) {
    /* 
        Function which generates constant expression, which basically
        equal to gen_const, but with some additional logic.
        Returns structure which represents the result of generation.
    */
    char buf[64];
    // allocating Const structure on stack
    // then it will be completed, and gen_const call will be performed
    Const const_expr;
    Type* expr_type = retrieve_expr_type(expr);
    const_expr.type = expr_type;
    if (is_integral_type(expr_type) || is_pointer_like_type(expr_type)) {
        if (is_unsigned_type(expr_type)) {
            const_expr.kind = CONST_UINT;
        } 
        else {
            if (is_pointer_like_type(expr_type)) {
                const_expr.kind = CONST_UINT;
            } else {
                const_expr.kind = CONST_INT;
            }
        }
        const_expr.ivalue = evaluate_expr_itype(expr);
    }
    else {
        // in this case evaluation is needed because
        // gen_const uses const->svalue for value, not const->fvalue
        double eval = evaluate_expr_ftype(expr);
        sprintf(buf, "%f", eval);
        char* heapval = cnew(char, sizeof buf);
        sprintf(heapval, "%s", buf);
        const_expr.fvalue = eval;
        const_expr.kind = CONST_FLOAT;
        const_expr.svalue = heapval;
        putalloc(heapval);
    }
    return gen_const(&const_expr);
}

gen_data gen_unary_expr(UnaryExpr* unary_expr) {
    /* 
        Function which generates unary expression.
        Returns structure which represents the result of generation.
    */
    TextInstruction* uninstr = NULL;
    gen_data data = gen_expr(unary_expr->expr);
    switch (unary_expr->kind) {
        // todo: logical not
        case UNARY_BW_NOT:
            uninstr = instr_un(NOT);
            break;
        case UNARY_PLUS:
        case UNARY_MINUS: {
            if (unary_expr->kind == UNARY_PLUS) {
                return data;
            }
            if (is_integral_type(unary_expr->type)) {
                uninstr = instr_un(NEG);
            } else {
                regid storage = reserve_any(REGV);
                TextInstruction* movinstr = mov_instr(unary_expr->type);
                movinstr->left = instr_arg_reg(storage);
                movinstr->right = instr_arg("__real@neg");
                movinstr->right->dereference = true;
                uninstr = instr_bin(XORPS);
                uninstr->right = instr_arg_reg(storage);
                unreserve(storage), puttext(movinstr);
            }
            break;
        }
        case UNARY_ADDRESS:
            uninstr = instr_bin(LEA);
            uninstr->right = instr_arg_reg(data._test_reg);
            uninstr->right->dereference = true;
            uninstr->right->offset = data._test_offset;
            break;
        case UNARY_DEREFERENCE:
            uninstr = instr_bin(MOV);
            uninstr->right = instr_arg_reg(data.reg);
            uninstr->right->dereference = true;
            uninstr->right->spec = get_size_spec(unary_expr->type);
            break;
        default:
            report_error(frmt("Unknown unary expression kind met"
                " in function: %s", __FUNCTION__), NULL);
    }
    uninstr->left = instr_arg_reg(data.reg);
    return puttext(uninstr), data;
}

gen_data gen_binary_expr(BinaryExpr* binary_expr) {
    ;
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

gen_data gen_stmt(Stmt* stmt) {
    gen_data data = {0};
    switch (stmt->kind) {
        case STMT_EXPR:
            return gen_expr_stmt(stmt->expr_stmt);
        case STMT_VAR_DECL:
            return gen_var_decl_stmt(stmt->var_decl);
        case STMT_FUNC_DECL:
            return gen_func_decl_stmt(stmt->func_decl);
        case STMT_TYPEDEF:
        case STMT_TYPE_DECL:
            break;
        default:
            report_error(frmt("Unknown stmt kind met"
                " in function: %s", __FUNCTION__), NULL);
    }
    return data;
}

gen_data gen_scope(Block* block) {
    gen_data data = {0};
    for (size_t i = 0; i < sbuffer_len(block->stmts); i++) {
        gen_data temp = gen_stmt(block->stmts[i]);
        merge_gen_data(&data, &temp);
    }
    return data;
}

gen_data gen_expr_stmt(ExprStmt* expr_stmt) {
    gen_data data = gen_expr(expr_stmt->expr);
    return unreserve(data.reg), data;
}

gen_data gen_var_decl_stmt(VarDecl* var_decl) {

    gen_data data = {0};
    if (var_decl->var_init != NULL) {
        // if variable is initialized with some expression
        // generate it and unreserve the register which was occupied
        data = gen_expr(var_decl->var_init);
        unreserve(data.reg);
    }
    StackFrameEntity* entity = glob_frame_get(var_decl->type_var->var);
    if (entity == NULL) {
        report_error(frmt("Cannot find stack frame entity with name %s, "
            "in %s", var_decl->type_var->var, __FUNCTION__), NULL);
    }
    Type* expr_type = retrieve_expr_type(var_decl->var_init);
    TextInstruction* movinstr = mov_instr(expr_type);
    movinstr->left = instr_arg_reg(RBP);
    movinstr->left->offset = entity->in.offset;
    movinstr->left->dereference = true;
    movinstr->left->spec = get_size_spec(var_decl->type_var->type);
    movinstr->right = instr_arg_reg(data.reg);
    puttext(movinstr);
    return data;
}

gen_data gen_func_decl_stmt(FuncDecl* func_decl) {
    // todo: generate case when function is external
    gen_data data = {0};
    StackFrame* frame = stack_frame_new(func_decl);
    glob_info.curr_frame = frame;
    if (func_decl->specs & FUNC_SPEC_ENTRY) {
        glob_info.curr_asm->global = frame->func_decl_def;
    }
    puttext(flabel(frame->func_decl_def));
    // function prologue
    TextInstruction* pr1 = instr_un(PUSH);
    pr1->left = instr_arg_reg(RBP);
    TextInstruction* pr2 = instr_bin(MOV);
    pr2->left = instr_arg_reg(RBP);
    pr2->right = instr_arg_reg(RSP);
    puttext(pr1), puttext(pr2);
    if (frame->reserved.locals != 0) {
        // do not allocate stack space if it is not needed
        TextInstruction* pr3 = instr_bin(SUB);
        pr3->left = instr_arg_reg(RSP);
        char* stack_size = cnew(char, 64);
        sprintf(stack_size, "%lu", frame->reserved.locals);
        pr3->right = instr_arg(stack_size);
        putalloc(stack_size), puttext(pr3);
    }
    // -----------------
    
    gen_data temp = gen_scope(func_decl->body);
    merge_gen_data(&data, &temp);

    // function epilogue
    if (frame->reserved.locals != 0) {
        // do not reset stack pointer if reserved stack space is zero
        TextInstruction* ep1 = instr_bin(MOV);
        ep1->left = instr_arg_reg(RSP);
        ep1->right = instr_arg_reg(RBP);
        puttext(ep1);
    }
    TextInstruction* ep2 = instr_un(POP);
    ep2->left = instr_arg_reg(RBP);
    TextInstruction* ep3 = instr(RET);
    puttext(ep2), puttext(ep3);
    // -----------------
    stack_frame_free(frame);
    return data;
}