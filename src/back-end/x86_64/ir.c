#include "ir.h"

#define CRLF "\r\n"
#define INDN "    "

#define putasm(fd, format, ...)                 \
    if (fprintf(fd, format, __VA_ARGS__) < 0) { \
        report_error(frmt("Runtime error in putasm macro %s:%d.", __FILE__, __LINE__), NULL);  \
    }

#define putasmn(fd, format, ...) putasm(fd, format CRLF, __VA_ARGS__)

void reserve(regid reg) {
    // in case of SIMD (SSE), there are no subregisters
    int subregs = is_simd_reg(reg) ? 1 : (RBX-RAX);
    // retrieveing register of the highest rank
    // (this will make all future register's state assigning more simplier)
    if (is_genp_reg(reg)) {
        // to avoid division by zero in case of SIMD register
        reg = reg - (reg % subregs);
    }
    for (int i = 0; i < subregs; i++) {
        switch (regtable[reg+i]) {
            case REG_FREE: _b(regtable[reg+i]=REG_RSVD);
            case REG_RSVD: _b(regtable[reg+i]=REG_PSHD);
            case REG_PSHD: 
                report_error2(frmt("Runtime error of register allocation "
                    "met in %s", __FUNCTION__), NULL);
                break;
            default:
                report_error2(frmt("Unknown register allocation command"
                    " met in %s", __FUNCTION__), NULL);
                break;
        }
    }
}

regstate unreserve(regid reg) {
    regstate prevst = regtable[reg];
    // in case of SIMD (SSE), there are no subregisters
    int subregs = is_simd_reg(reg) ? 1 : (RBX-RAX);
    // retrieveing register of the highest rank
    // (this will make all future register's state assigning more simplier)
    if (is_genp_reg(reg)) {
        // to avoid division by zero in case of SIMD register
        reg = reg - (reg % subregs);
    }
    for (int i = 0; i < subregs; i++) {
        switch (regtable[reg+i]) {
            case REG_FREE: _b(;);
            case REG_RSVD: _b(regtable[reg+i]=REG_FREE);
            case REG_PSHD: _b(regtable[reg+i]=REG_RSVD);
            default:
                report_error2(frmt("Unknown register allocation command"
                    " met in %s", __FUNCTION__), NULL);
                break;
        }
    }
    // return previous state, it is needed for 
    // calling environment to understand if code 
    // for register retrieveing from stack is needed  
    return prevst;
}

regid reserve_any(regtype type) {
    if (type == REGV) {
        for (int attempt = 0; attempt < 2; attempt++) {
            // at the first time alghorithm will search for
            // free registers, at the second turn, 
            // all reserved registers will be under review 
            for (regid xmm = XMM0; xmm <= XMM15; xmm++) {
                if (regtable[xmm] == REG_FREE || (attempt == 1 && regtable[xmm] == REG_RSVD)) {
                    return reserve(xmm), xmm;
                }
            }
        }
        // if there are no available registers, report error
        report_error2(frmt("Runtime error of SIMD register allocation,"
            " met in %s", __FUNCTION__), NULL);
    }
    else {
        int offset = 0;
        int iterator = (RBX-RAX);
        switch (type) {
            case REGB: _b(offset=3);
            case REGW: _b(offset=2);
            case REGD: _b(offset=1);
            case REGQ: _b(offset=0);
            default:
                report_error2(frmt("Unknown register type "
                    "met in %s", __FUNCTION__), NULL);
                break;
        }
        for (int attempt = 0; attempt < 2; attempt++) {
            // at the first time alghorithm will search for
            // free registers, at the second turn, 
            // all reserved registers will be under review 
            for (regid reg = RAX+offset; reg <= R15+offset; reg+=iterator) {
                if (regtable[reg] == REG_FREE || (attempt == 1 && regtable[reg] == REG_RSVD)) {
                    return reserve(reg), reg;
                }
            }
        }
        // if there are no available registers, report error
        report_error2(frmt("Runtime error of general purpose register allocation,"
            " met in %s", __FUNCTION__), NULL);
    }
}

regid get_reg_part(regid reg, InstructionArgumentSize size) {
    if (is_simd_reg(reg)) {
        return reg;
    }
    switch (size) {
        case SPEC_NONE:  return reg;
        case SPEC_BYTE:  return (regid)(reg+3);
        case SPEC_WORD:  return (regid)(reg+3);
        case SPEC_DWORD: return (regid)(reg+3);
        case SPEC_QWORD: return (regid)(reg+0);
        default:
            report_error(frmt("Unknown spec kind met"
                " in function: %s", __FUNCTION__), NULL);
    }
}

TextInstruction* label() {
    return flabel(alloc_label_def());
}

TextInstruction* flabel(char* label) {
    TextInstruction* instr = instr_un(LABEL);
    instr->left = instr_arg(label);
    return instr;
}

TextInstruction* comment(char* comment) {
    TextInstruction* instr = instr_un(COMMENT);
    instr->left = instr_arg(comment);
    return instr;
}

void text_instrarg_to_stream(TextInstructionArg* text_instrarg, FILE* fd) {
    switch (text_instrarg->spec) {
        case SPEC_NONE: _b(;);
        case SPEC_BYTE:
        case SPEC_WORD:
        case SPEC_DWORD:
        case SPEC_QWORD:
            _b(putasm(fd, "%s ", instr_spec_tostr(text_instrarg->spec)));
        default:
            report_error(frmt("Unknown spec kind met"
                " in function: %s", __FUNCTION__), NULL);
    }
    if (text_instrarg->dereference) {
        putasm(fd, "%s", "[");
    }
    putasm(fd, "%s", text_instrarg->value);
    if (text_instrarg->offset != 0) {
        if (text_instrarg->offset > 0) {
            putasm(fd, "+%ld", text_instrarg->offset);
        } else {
            putasm(fd, "-%ld", -text_instrarg->offset);
        }
    }
    if (text_instrarg->dereference) {
        putasm(fd, "%s", "]");
    }
}

void text_instr_to_stream(TextInstruction* text_instr, FILE* fd) {
    switch (text_instr->instruction) {
        case LABEL:    _b(putasmn(fd, INDN "%s:", text_instr->left->value));
        case COMMENT:  _b(putasmn(fd, INDN "; %s", text_instr->left->value));
        default:
            putasm(fd, INDN INDN "%-10s", instr_tostr(text_instr->instruction));
            switch (text_instr->kind) {
                case INSTRUCTION_NOP:       
                    break;
                case INSTRUCTION_UNARY:
                    text_instrarg_to_stream(text_instr->left, fd);
                    break;
                case INSTRUCTION_BINARY:
                    text_instrarg_to_stream(text_instr->left, fd);
                    putasm(fd, "%s ", ",");
                    text_instrarg_to_stream(text_instr->right, fd);
                    break;
            }
            putasm(fd, "%s", CRLF);
            break;
    }
}

void text_seg_to_stream(Assembly* assembly, TextSegment* text_seg, FILE* fd) {
    putasmn(fd, "%s", "section" " .text");
    putasmn(fd, INDN "%s %s", "global", assembly->global);
    // todo: external functions
    if (text_seg != NULL) {
        for (size_t i = 0; i < sbuffer_len(text_seg->instructions); i++) {
            text_instr_to_stream(text_seg->instructions[i], fd);
        }
    }
}

void data_instr_to_stream(DataInstruction* data_instr, FILE* fd) {
    putasm(fd, "\t%s", data_instr->name);
    if (data_instr->times) {
        putasm(fd, ": times %ld", data_instr->times_value);
    }
    putasm(fd, data_instr->times ? " %s " : "\t%s ", instr_size_tostr(data_instr->size));
    for (size_t i = 0; i < sbuffer_len(data_instr->value); i++) {
        putasm(fd, "%s", data_instr->value[i]);
        if (i < sbuffer_len(data_instr->value)-1) {
            putasm(fd, "%s ", ",");
        }
    }
    putasm(fd, "%s", CRLF);
}

void data_seg_to_stream(DataSegment* data_seg, FILE* fd) {
    putasmn(fd, CRLF "%s", "section" " .data");
    if (data_seg != NULL) {
        for (size_t i = 0; i < sbuffer_len(data_seg->instructions); i++) {
            data_instr_to_stream(data_seg->instructions[i], fd);
        }
    }
}

void bss_seg_to_stream(BssSegment* bss_seg, FILE* fd) {
    putasmn(fd, CRLF "%s", "section" " .bss");
    if (bss_seg != NULL) {
        for (size_t i = 0; i < sbuffer_len(bss_seg->instructions); i++) {
            data_instr_to_stream(bss_seg->instructions[i], fd);
        }
    }
}

void asm_to_stream(Assembly* assembly, FILE* fd) {
    text_seg_to_stream(assembly, assembly->text, fd);
    data_seg_to_stream(assembly->data, fd);
    bss_seg_to_stream(assembly->bss, fd);
}

void predef_data_instrs(DataSegment* data_seg) {
    /* 
        Function that specifies some useful constants
        in data segment, which will be used by generator
    */

    // dobule precision float constant for negation of xmm registers
    char** values = NULL;
    sbuffer_add(values, "8000000000000000h");  
    DataInstruction* data_instr = data_instr("__real@neg", values,
        SPEC_QWORD, DATA_FLOAT_CONST);
    sbuffer_add(data_seg->instructions, data_instr);
    // ------------------------------
}

TextSegment* text_seg_new() {
    TextSegment* text_seg = new(TextSegment);
    text_seg->instructions = NULL;
    return text_seg;
}

TextInstruction* text_instr_new(unsigned instr, TextInstructionKind kind, TextInstructionArg* lop, TextInstructionArg* rop) {
    TextInstruction* text_instr = new(TextInstruction);
    text_instr->kind = kind;
    text_instr->left = lop;
    text_instr->right = rop;
    text_instr->instruction = instr;
    return text_instr;
}

TextInstructionArg* text_instrarg_new(const char* value, InstructionArgumentSize spec, long offset, bool dereference) {
    TextInstructionArg* text_instrarg = new(TextInstructionArg);
    text_instrarg->spec = spec;
    text_instrarg->value = value;
    text_instrarg->offset = offset;
    text_instrarg->dereference = dereference;
    return text_instrarg;
}

BssSegment* bss_seg_new() {
    BssSegment* bss_seg = new(BssSegment);
    bss_seg->instructions = NULL;
    return bss_seg;
}

DataSegment* data_seg_new() {
    DataSegment* data_seg = new(DataSegment);
    data_seg->instructions = NULL;
    predef_data_instrs(data_seg);
    return data_seg;
}

DataInstruction* data_instr_new(char* name, char** value, DataInstructionKind kind, InstructionArgumentSize size, bool times, long times_value) {
    DataInstruction* data_instr = new(DataInstruction);
    data_instr->name = name;
    data_instr->size = size;
    data_instr->kind = kind;
    data_instr->value = value;
    data_instr->times = times;
    data_instr->times_value = times_value;
    return data_instr;
}

Assembly* asm_new() {
    Assembly* assembly = new(Assembly);
    assembly->bss = bss_seg_new();
    assembly->data = data_seg_new();
    assembly->text = text_seg_new();
    assembly->global = NULL;
    return assembly;
}

void text_seg_free(TextSegment* text_seg) {
    if (text_seg != NULL) {
        for (size_t i = 0; i < sbuffer_len(text_seg->instructions); i++) {
            text_instr_free(text_seg->instructions[i]);
        }
        sbuffer_free(text_seg->instructions);
        free(text_seg);
    }
}

void text_instr_free(TextInstruction* text_instr) {
    if (text_instr != NULL) {
        text_instrarg_free(text_instr->left);
        text_instrarg_free(text_instr->right);
        free(text_instr);
    }
}

void text_instrarg_free(TextInstructionArg* text_instr_arg) {
    if (text_instr_arg != NULL) {
        // todo: check if i need to free or not
        free(text_instr_arg);
    }
}

void bss_seg_free(BssSegment* bss_seg) {
    if (bss_seg != NULL) {
        for (size_t i = 0; i < sbuffer_len(bss_seg->instructions); i++) {
            data_instr_free(bss_seg->instructions[i]);
        }
        sbuffer_free(bss_seg->instructions);
        free(bss_seg);
    }
}

void data_seg_free(DataSegment* data_seg) {
    if (data_seg != NULL) {
        for (size_t i = 0; i < sbuffer_len(data_seg->instructions); i++) {
            data_instr_free(data_seg->instructions[i]);
        }
        sbuffer_free(data_seg->instructions);
        free(data_seg);
    }
}

void data_instr_free(DataInstruction* data_instr) {
    if (data_instr != NULL) {
        // todo: same thing as in text_instrarg_free
        free(data_instr);
    }
}

void asm_free(Assembly* assembly) {
    if (assembly != NULL) {
        // todo: free(assembly->global) ?
        bss_seg_free(assembly->bss);
        data_seg_free(assembly->data);
        text_seg_free(assembly->text);
        free(assembly);
    }
}