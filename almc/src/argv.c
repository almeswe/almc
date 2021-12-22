#include "argv.h"

ComplilationOptions* options_new()
{
	return cnew_s(ComplilationOptions,
		options, 1);
}

void parse_compiler_roots(ComplilationOptions* options)
{
	// setting up the compiler's root folder
	char* buffer[MAX_PATH];
	GetModuleFileNameA(NULL, buffer, MAX_PATH);
	options->compiler.root = get_dir_parent(buffer);
	// setting up paths to linker and masm compiler
	options->compiler.ml_path = frmt("%s\\%s",
		options->compiler.root, MASM_COMPILER);
	options->compiler.link_path = frmt("%s\\%s",
		options->compiler.root, MASM_LINKER);
	// setting up paths to lib and inc folders
	options->compiler.inc_path = frmt("%s\\%s",
		options->compiler.root, MASM_SDK_INC_FOLDER);
	options->compiler.lib_path = frmt("%s\\%s",
		options->compiler.root, MASM_SDK_LIB_FOLDER);
}

void parse_target_roots(ComplilationOptions* options, 
	char* target_path, char* output_name)
{
	// setting up the root folder of source file
	options->target.root = 
		get_dir_parent(target_path);
	// setting up paths to target source file, and its outputs
	options->target.target_path = target_path;
	options->target.asm_path = frmt("%s\\%s.asm", 
		options->target.root, output_name);
	options->target.object_path = frmt("%s\\%s.obj",
		options->compiler.root, output_name);
}

ComplilationOptions* parse_options(char** argv, int argc)
{
	ComplilationOptions* options = 
		options_new();

	if (argc != 3)
		report_error(frmt("Wrong argument count passed: %d, expected: %d arguments", 
			argc, 3), NULL);

	parse_compiler_roots(options);
	parse_target_roots(options, argv[1], argv[2]);
	return options;
}

void options_free(ComplilationOptions* options)
{
	return;
}