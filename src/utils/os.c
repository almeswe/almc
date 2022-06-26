#include "os.h"

#ifdef _WIN32
	#error "this code must be without malloc!"
	#define PATH_SEPARATOR '\\'

	char* get_drives() {
		char* drives[MAX_PATH];
		GetLogicalDriveStringsA(MAX_PATH, drives);
		GetLogicalDrives(MAX_PATH, drives);
		return drives;
	}

	char* get_curr_dir() {
		char* buffer[MAX_PATH];
		GetCurrentDirectoryA(MAX_PATH, buffer);
		return buffer;
	}

	char* get_curr_module() {
		char* buffer[MAX_PATH];
		GetModuleFileNameA(NULL, buffer, MAX_PATH);
		return buffer;
	}

	char* get_root(char* path) {
		char* disk = (char*)malloc(sizeof(char) * 4);
		if (!disk) {
			return NULL;
		}
		disk[0] = path[0];
		disk[1] = ':';
		disk[2] = '\\';
		disk[3] = '\0';
		return disk;
	}

	char* get_dir_parent(char* dir) {
		size_t dir_name_len = 0;
		size_t dir_len = strlen(dir);
		for (size_t i = dir_len - 1; i > 0; i--, dir_name_len++)
			if (dir[i] == PATH_SEPARATOR)
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

	char* path_combine(char* parent, char* child) {
		size_t child_size = strlen(child);
		size_t parent_size = strlen(parent);
		char ends_with_sep =
			parent[parent_size - 1] == PATH_SEPARATOR;
		size_t combined_size = child_size + parent_size + 
			1 + !ends_with_sep; // +2 -> 1 for separator & 1 for \0
		char* combined = (char*)malloc(combined_size * sizeof(char)); 
		if (!combined)
			return NULL;
		strcpy(combined, parent);
		if (!ends_with_sep)
			combined[parent_size] = PATH_SEPARATOR;
		strcpy(combined + parent_size + !ends_with_sep, child);
		return combined;
	}

	bool dir_exists(char* dir) {
		return GetFileAttributes(dir) == 
			FILE_ATTRIBUTE_DIRECTORY;
	}

	bool file_exists(char* file) {
		FILE* f = NULL;
		fopen_s(&f, file, "r");
		if (!f) {
			return false;
		}
		fclose(f);
		return true;
	}
#elif __linux__

	#define PATH_SEPARATOR '/'
	#define _SHARED_BUFFER_SIZE 1024
	char _shared_buffer[_SHARED_BUFFER_SIZE];

	char* get_drives() {
		return NULL;
	}

	char* get_curr_dir() {
		if (getcwd(_shared_buffer, sizeof _shared_buffer) != NULL) {
			return _shared_buffer;
		}
		return NULL;
	}

	char* get_curr_module() {
		return NULL;
	}

	char* get_root(char* path) {
		return "/";
	}

	char* get_dir_parent(char* dir) {
		return dirname(dir);
	}

	char* path_combine(char* parent, char* child) {
		if (parent[strlen(parent)-1] == PATH_SEPARATOR) {
			if (sprintf(_shared_buffer, "%s%s", parent, child) < 0) {
				return NULL;
			}
		} 
		else {
			if (sprintf(_shared_buffer, "%s%c%s", 
					parent, PATH_SEPARATOR, child) < 0) {
				return NULL;
			}
		}
		return _shared_buffer;
	}

	bool dir_exists(char* path) {
		DIR* dir = opendir(path);
		if (dir != NULL) {
			return closedir(dir), true;
		}
		return false;
	}

	bool file_exists(char* path) {
		return access(path, F_OK) == 0;
	}

#endif