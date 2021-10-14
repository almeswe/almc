#include "os.h"

#define WIN_PATH_SEP '\\'

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
	size_t dir_name_len = 0;
	size_t dir_len = strlen(dir);
	for (size_t i = dir_len - 1; i > 0; i--, dir_name_len++)
		if (dir[i] == WIN_PATH_SEP)
			break;
	char* parent = (char*)calloc(
		dir_len - dir_name_len - 1, sizeof(char));
	if (!parent)
		return NULL;
	for (size_t i = 0; i < dir_len - dir_name_len; i++)
		parent[i] = dir[i];
	parent[dir_len - dir_name_len - 1] = '\0';
	return parent;
}

char* path_combine(const char* parent, const char* child)
{
	size_t child_size = strlen(child);
	size_t parent_size = strlen(parent);
	size_t combined_size = child_size + parent_size + 2; // +2 -> 1 for separator & 1 for \0
	char* combined = (char*)malloc((parent_size +
		child_size + 2) * sizeof(char)); 
	if (!combined)
		return NULL;
	strcpy_s(combined, combined_size, parent);
	char ends_with_sep = 
		parent[parent_size - 1] == WIN_PATH_SEP;
	if (!ends_with_sep)
		combined[parent_size] = WIN_PATH_SEP;
	strcpy_s(combined + parent_size + !ends_with_sep,
		 combined_size, child);
	return combined;
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