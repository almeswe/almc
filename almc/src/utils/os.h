#ifndef ALMC_OS_H
#define ALMC_OS_H

#define WIN32_LEAN_AND_MEAN
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

char* get_drives();
char* get_curr_dir();
char* get_dir_parent(const char* dir);

char* path_combine(const char* parent, const char* child);

int dir_exists(const char* dir);
int file_exists(const char* file);

#endif