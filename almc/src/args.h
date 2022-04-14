#ifndef _ALMC_COMPILATION_OPTIONS_H
#define _ALMC_COMPILATION_OPTIONS_H

#include "error.h"
#include "utils/os.h"
#include "utils/common.h"

#define MASM_LINKER "bin\\link"
#define MASM_COMPILER "bin\\ml"

#define MASM_SDK_BIN_FOLDER "bin"
#define MASM_SDK_LIB_FOLDER "lib"
#define MASM_SDK_INC_FOLDER "inc"

struct CompilerRoots
{
	char* root;
	
	char* lib_path;
	char* inc_path;

	char* ml_path;
	char* link_path;
};

struct TargetRoots
{
	char* root;

	char* asm_path;
	char* target_path;
	char* object_path;
	char* binary_path;

	char* src_object_path;
	char* src_binary_path;
};

typedef struct {
	struct TargetRoots target;
	struct CompilerRoots compiler;
} ComplilationOptions;

ComplilationOptions* options;

ComplilationOptions* options_new();
ComplilationOptions* parse_options(char** argv, int argc);
void options_free(ComplilationOptions* options);

#endif // _ALMC_COMPILATION_OPTIONS_H