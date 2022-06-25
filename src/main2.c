#include <stdio.h>

#include "error.h"
#include "printerr.h"

int main(int argc, char** argv) {
    SrcContext* context = src_context_new("src/front-end/lexer.h", 10, 5, 101);
    report_warning("test error", context);
    src_context_free(context);    
    return 0;
}