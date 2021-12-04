#ifndef ALMC_BACK_END_TYPES
#define ALMC_BACK_END_TYPES

#define REAL4 4
#define REAL8 5

#define BYTE 0
#define WORD 1
#define DWORD 2
#define QWORD 3

#include "..\..\front-end\ast\ast.h"

int get_type_size(Type* type);
int get_type_prefix(Type* type);
const char* get_predefined_type_str(int prefix);

#endif