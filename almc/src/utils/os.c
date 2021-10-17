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

char* get_root(const char* path)
{
	//todo: this function is really unsafe
	char* disk = (char*)malloc(sizeof(char) * 4);
	if (!disk)
		return NULL;
	disk[0] = path[0];
	disk[1] = ':';
	disk[2] = '\\';
	disk[3] = '\0';
	return disk;
}

char* get_dir_parent(const char* dir)
{
	size_t dir_name_len = 0;
	size_t dir_len = strlen(dir);
	for (size_t i = dir_len - 1; i > 0; i--, dir_name_len++)
		if (dir[i] == WIN_PATH_SEP)
			break;
	char* parent = (char*)malloc(sizeof(char) * 
		(dir_len - dir_name_len));
	if (!parent)
		return NULL;
	for (size_t i = 0; i < dir_len - dir_name_len - 1; i++)
		parent[i] = dir[i];
	parent[dir_len - dir_name_len - 1] = '\0';
	return parent;
}

char* path_combine(const char* parent, const char* child)
{
	size_t child_size = strlen(child);
	size_t parent_size = strlen(parent);
	char ends_with_sep =
		parent[parent_size - 1] == WIN_PATH_SEP;
	size_t combined_size = child_size + parent_size + 
		1 + !ends_with_sep; // +2 -> 1 for separator & 1 for \0
	char* combined = (char*)malloc(combined_size * sizeof(char)); 
	if (!combined)
		return NULL;
	strcpy(combined, parent);
	if (!ends_with_sep)
		combined[parent_size] = WIN_PATH_SEP;
	strcpy(combined + parent_size + !ends_with_sep, child);
	return combined;
}

int dir_exists(const char* dir)
{
	return GetFileAttributes(dir) == 
		FILE_ATTRIBUTE_DIRECTORY;
}

int file_exists(const char* file)
{
	FILE* f = 0;
	fopen_s(&f, file, "r");
	if (!f)
		return 0;
	fclose(f);
	return 1;
}