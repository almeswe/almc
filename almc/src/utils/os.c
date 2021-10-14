#include "os.h"

char* get_drives()
{
	char* drives[MAX_PATH];
	GetLogicalDriveStringsA(MAX_PATH, drives);
	GetLogicalDrives(MAX_PATH, drives);
	return drives;
}

char* get_curr_dir()
{
	char dir[MAX_PATH];
	GetCurrentDirectoryA(MAX_PATH, dir);
	return dir;
}

char* get_dir_parent(const char* dir)
{
	char* parent;
	size_t dir_name_len = 0;
	size_t dir_len = strlen(dir);
	for (size_t i = dir_len - 1; i > 0; i--, dir_name_len++)
		if (dir[i] == '\\' || dir[i] == '/')
			break;
	if (!(parent = (char*)calloc(dir_len - dir_name_len - 1, sizeof(char))))
		return NULL;
	for (size_t i = 0; i < dir_len - dir_name_len; i++)
		parent[i] = dir[i];
	parent[dir_len - dir_name_len - 1] = '\0';
	return parent;
}

int dir_exists(const char* dir)
{
	return GetFileAttributesA(dir) == 
		FILE_ATTRIBUTE_DIRECTORY;
}

int file_exists(const char* file)
{
	DWORD dwAttrib = GetFileAttributesA(file);
	return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
		!(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}