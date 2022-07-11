#ifndef _ALMC_STRING_BUILDER_H
#define _ALMC_STRING_BUILDER_H

#define STR_BUILDER_BUFFER_SIZE 128

#define str_eq(str1, str2) \
	((str1 != NULL && str2 != NULL) && (strcmp(str1, str2) == 0))

#define str_builder_create_buffer(buffer) \
	char* buffer = newc(char, STR_BUILDER_BUFFER_SIZE)

#define str_builder_reduce_buffer(temp, buffer, size) \
	char* temp = buffer;							  \
	if (!(buffer = rnew(char, size + 1, buffer)))	  \
		 buffer = temp;								  \
	buffer[size] = '\0';

#define str_builder_err										    \
	report_error(frmt("Identifier size should be less than %d", \
		STR_BUILDER_BUFFER_SIZE), NULL)

#define str_builder_add(buffer, digit, index)			      \
	((index)+1 > STR_BUILDER_BUFFER_SIZE) ? str_builder_err : \
		(buffer[index] = digit)

#endif // _ALMC_STRING_BUILDER_H