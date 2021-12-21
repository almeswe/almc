#include "argv.h"

#define MASM_COMPILER "ml"
#define MASM_LINKER "link"

#define MASM_SDK_BIN_FOLDER "masm32\\bin"
#define MASM_SDK_LIB_FOLDER "masm32\\lib"
#define MASM_SDK_INC_FOLDER "masm32\\includes"

ComplilationOptions* options_new(char* argv0)
{
	ComplilationOptions* options = cnew_s(ComplilationOptions,
		options, 1);
	options->root = get_dir_parent(argv0);
	if (!options->root)
		options->root = "";
	return options;
}

ComplilationOptions* parse_options(char** argv, int argc)
{
	ComplilationOptions* options = options_new(argv[0]);

	if (argc != 3)
		report_error(frmt("Wrong argument count passed: %d, expected: %d arguments", 
			argc, 3), NULL);

	options->target_path = frmt(argv[1]);

	// creating the path for obj and asm files
	options->asm_path = frmt("%s\\%s.asm",
		options->root, argv[2]);
	options->obj_path = frmt("%s\\%s.obj",
		options->root, argv[2]);
	// setting the masm sdk path to libs and includes?
	options->libpath = frmt("%s\\%s", 
		options->root, MASM_SDK_LIB_FOLDER);
	options->incpath = frmt("%s\\%s", 
		options->root, MASM_SDK_INC_FOLDER);
	// swtting linker and compiler of masm
	options->linker_path = frmt("%s\\%s\\%s", 
		options->root, MASM_SDK_BIN_FOLDER, MASM_LINKER);
	options->masm_ml_path = frmt("%s\\%s\\%s",
		options->root, MASM_SDK_BIN_FOLDER, MASM_COMPILER);
	return options;
}

void options_free(ComplilationOptions* options)
{
	return;
}