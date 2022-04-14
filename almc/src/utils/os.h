#ifndef _ALMC_OS_H
#define _ALMC_OS_H

#ifdef _WIN32

	#define WIN32_LEAN_AND_MEAN
	#define _CRT_SECURE_NO_WARNINGS

	#include <stdio.h>
	#include <stdlib.h>
	#include <windows.h>

	char* get_drives();
	char* get_curr_dir();
	char* get_curr_module();
	char* get_root(const char* path);
	char* get_dir_parent(const char* dir);

	char* path_combine(const char* parent, const char* child);

	int dir_exists(const char* dir);
	int file_exists(const char* file);

	#undef _CRT_SECURE_NO_WARNINGS

#else
	#error "Os module is not supported on this platform."
#endif

#endif // _ALMC_OS_H