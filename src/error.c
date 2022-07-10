#include "error.h"

#define SPELL_ERR 		"Error"
#define SPELL_WARN 		"Warning"
#define SPELL_TABSIZE 	8

#define BLOCK_SIZE 4096

#define SPELL_YEL   "\x1B[33m"
#define SPELL_RED   "\x1B[31m"
#define SPELL_RESET "\x1B[0m"

sp_data* get_spelling(const SrcContext* context) {
	if (!context) {
		return NULL;
	}
	FILE* fd = NULL;
	if ((fd = fopen(context->file, "r")) == NULL) {
		return NULL;
	}
	char buffer[4096];
	memset(buffer, 0, sizeof buffer);

	for (size_t i = 1; i <= context->line; i++) {
		if (fgets(buffer, sizeof buffer, fd) == NULL) {
			if (!feof(fd)) {
				return fclose(fd), NULL;
			}
		}
		if (i != context->line) {
			memset(buffer, 0, sizeof buffer);
		}
	}
	fclose(fd);
	size_t length = strlen(buffer);
	sp_data* data = new(sp_data);
	data->target = cnew(char, length+1);
	data->marked = cnew(char, length+1);
	strcpy(data->target, buffer);

	size_t margin = 0;
	for (size_t i = 0; i < length; i++) {
		margin += (buffer[i] == '\t') ? SPELL_TABSIZE : 1;
	}
	for (size_t i = 0; i < context->start; i++) {
		data->marked[i] = ' ';
	}
	for (size_t i = 0; i < context->size; i++) {
		data->marked[i+context->start-1] = '~';
	}
	return data;
}

void free_spelling(sp_data* spelling) {
	if (spelling != NULL) {
		free(spelling->target);
		free(spelling->marked);
		free(spelling);
	}
}

void report_info_base(const char* header, const char* message, sp_data* spelling,
	any_context_to_str to_str_func, const void* any_context) {
	// printing header [error, warning] and message
	printerr("%s: %s ", header, message);
	// printing context if there are any
	printerr("%s\n", any_context ? to_str_func(any_context) : "");
	// and finally printing spelling lines if there are any
	if (spelling) {
		printerr("\n%s\n%s\n", spelling->target, spelling->marked);
		free_spelling(spelling);
	}
}

void report_warning(const char* message, const SrcContext* context) {
	report_info_base(SPELL_YEL SPELL_WARN SPELL_RESET, message, get_spelling(context),
		(any_context_to_str*)src_context_tostr, context);
}

void report_warning2(const char* message, const SrcArea* area) {
	report_info_base(SPELL_YEL SPELL_WARN SPELL_RESET, message, get_spelling(area->begins),
		(any_context_to_str*)src_area_tostr, area);
}

void report_error(const char* message, const SrcContext* context) {
	report_info_base(SPELL_RED SPELL_ERR SPELL_RESET, message, get_spelling(context),
		(any_context_to_str*)src_context_tostr, context);
	exit(1);
}

void report_error2(const char* message, const SrcArea* area) {
	report_info_base(SPELL_RED SPELL_ERR SPELL_RESET, message, get_spelling(area->begins),
		(any_context_to_str*)src_area_tostr, area);
	exit(1);
}