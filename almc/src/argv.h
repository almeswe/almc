#ifndef ALMC_COMPILATION_OPTIONS
#define ALMC_COMPILATION_OPTIONS

#include "error.h"
#include "utils/os.h"
#include "utils/common.h"

typedef struct {
	char* root;

	char* obj_path;
	char* asm_path;

	char* target_path;

	char* libpath;
	char* incpath;

	char* linker_path;
	char* masm_ml_path;
} ComplilationOptions;

ComplilationOptions* options;

ComplilationOptions* options_new(char* argv0);
ComplilationOptions* parse_options(char** argv, int argc);
void options_free(ComplilationOptions* options);

#endif