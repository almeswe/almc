#include "common.h"

size_t _frmt_buffer_ptr = 0;

size_t _get_frmt_buffer_ptr() {
	return _frmt_buffer_ptr;
}

char* frmt(const char* format, ...) {
	/*
		Function which returns new formatted string
		according to passed parameters.
	*/

	va_list args;
	size_t written = 0;
	va_start(args, format);
	if ((written = vsprintf(_frmt_buffer+_frmt_buffer_ptr, format, args)) < 0) {
		printerr("%s\n", "vsprintf in frmt function errored."); exit(1);
	}
	va_end(args);
	written += 1;
	_frmt_buffer_ptr += written;
	if (_frmt_buffer_ptr >= (size_t)(_FRMT_BUFFER_SIZE*_FRMT_BUFFER_MAX_LOAD)) {
		va_start(args, format);
		_frmt_buffer_ptr = 0;
		if ((written = vsprintf(_frmt_buffer, format, args)) < 0) {
			printerr("%s\n", "vsprintf in frmt function errored."); exit(1);
		}
		va_end(args);
		_frmt_buffer_ptr += written;
	}
	return _frmt_buffer+_frmt_buffer_ptr-written;
}

bool isidnt(char c) {
	/*
		Checks if the specified character is possible for identifier lexeme.
	*/
	return isalpha(c) || c == '_';
}

bool isidnt_ext(char c) {
	/*
		Checks for 'extended' identifier character pattern.
		Digits included.
	*/
	return isdigit(c) || isidnt(c);
}

bool isdigit_bin(char c) {
	/*
		Checks if the specified character has binary format.
	*/
	return c == '0' || c == '1';
}

bool isdigit_oct(char c) {
	/*
		Checks if the specified character has octal format.
	*/
	return isdigit(c) || (c >= '0' && c <= '7');
}

bool isdigit_hex(char c) {
	/*
		Checks if the specified character has hexidecimal format.
		Includes both lower and upper.
	*/
	return isdigit(c) || ((c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'));
}

bool isdigit_ext(char c) {
	/*
		Checks the 'extended' digit pattern for numeric lexemes.
		[e, E] - exponential form.
		[.]    - floating point number.
		[_]    - digit separator (i.e. 1_000_000).
	*/
	return isdigit(c) || c == '.' || c == 'e' || c == 'E' ||
		c == '_';
}

bool isdigit_fext(char c) {
	/*
		Checks the 'extended' digit pattern for numeric lexemes.
		[e, E] - exponential form.
	*/
	return isdigit(c) || c == 'e' || c == 'E';
}

bool isstrc(char c) {
	/*
		Checks character which means the end of string lexeme.
	*/
	return c != '\n' && c != '\"';
}

bool issquote(char c) {
	return c == '\'';
}

bool isdquote(char c) {
	return c == '\"';
}

bool issharp(char c) {
	return c == '#';
}

bool isescape(char c) {
	/*
		Checks for all available (for this compiler) escape characters.
	*/
	switch (c) {
		case '\a':
		case '\b':
		case '\f':
		case '\n':
		case '\r':
		case '\t':
		case '\v':
			return true;
	}
	return false;
}