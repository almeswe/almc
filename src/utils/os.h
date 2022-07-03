#ifndef _ALMC_OS_H
#define _ALMC_OS_H

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#define _CRT_SECURE_NO_WARNINGS
		#include <stdlib.h>
		#include <windows.h>
	#undef _CRT_SECURE_NO_WARNINGS
	#undef WIN32_LEAN_AND_MEAN
	#define PATH_SEPARATOR '\\'
#elif __linux__
	#include <unistd.h>
	#include <libgen.h>
	#include <dirent.h>
	#define PATH_SEPARATOR '/'
#else
	#error "this module is not supported on this platform."
#endif

char* get_drives();
char* get_curr_dir();
char* get_curr_module();
char* get_root(char* path);
char* get_dir_parent(char* dir);

char* path_combine(char* parent, char* child);

bool dir_exists(char* path);
bool file_exists(char* path);

#endif // _ALMC_OS_H