#include "lexer.h"

#define curr_char_fits(l) 		((l->stream - l->stream_origin) >= 0) && (!eos(l))
#define context_unpack(l, size) l->curr_file, l->curr_line_offset, size, l->curr_line

#define matchc(l, c)			(get_curr_char(l) == c)
#define matchc_in(l, c1, c2)	((get_curr_char(l)) >= (c1) && (get_curr_char(l)) <= (c2))

char chars[] = {
	'+', '-', '*', '/',
	'%', '|', '~', '^',
	'\'', '"', '!', ',',
	'.', ':', ';', '=',
	'?', '&', ')', '(',
	']', '[', '}', '{', '<', '>'
};

char* keychars[] = {
	"+=", "-=", "*=", "/=",
	"%=", "<<=", ">>=", "|=",
	"&=", "^=", "~=", "||",
	"==", "!=", "&&", "<<",
	">>", "<=", ">=", "->",
	"..", "...", "./", "../"
};

char* keywords[] = {
	"break",
	"case",
	CHAR_TYPE,
	"cast",
	"continue",
	"default",
	"enum",
	"elif",
	"entry",
	"for",
	"fnc",
	"from",
	"false",
	F32_TYPE,
	F64_TYPE,
	"goto",
	"if",
	I8_TYPE,
	I16_TYPE,
	I32_TYPE,
	I64_TYPE,
	"import",
	"return",
	"struct",
	"switch",
	"sizeof",
	"true",
	"typedef",
	"union",
	U8_TYPE,
	U16_TYPE,
	U32_TYPE,
	U64_TYPE,
	"let",
	"label",
	"lengthof",
	"var",
	VOID_TYPE,
	"while",
	"do",
	"else"
};

bool bos(Lexer* lexer) {
	// beginning of stream
	return (lexer->stream - lexer->stream_origin) <= 0;
}

bool eos(Lexer* lexer) {
	// end of stream
	return (lexer->stream - lexer->stream_origin) >= lexer->stream_size;
}

int isknch(const char ch) {
	for (int i = 0; i < CHARS; i++) {
		if (ch == chars[i]) {
			return i;
		}
	}
	return -1;
}

int isextch(const char* ech) {
	for (int i = 0; i < CMP_CHARS; i++) {
		if (str_eq(ech, keychars[i])) {
			return i;
		}
	}
	return -1;
}

int iskeyword(const char* idnt) {
	for (int i = 0; i < KEYWORDS; i++) {
		if (str_eq(idnt, keywords[i])) {
			return i;
		}
	}
	return -1;
}

Lexer* lexer_new(char* input, StreamType input_type) {
	Lexer* lexer = new(Lexer);
	if ((lexer->input = input_type) == FROM_CHAR_PTR) {
		lexer->stream_origin = input;
		lexer->stream_size = strlen(lexer->stream_origin);
		lexer->curr_file = "undefined";
	}
	else {
		FILE* fd = NULL;
		if ((fd = fopen(input, "rb")) == NULL) {
			report_error(frmt("Cannot open file: \'%s\'", input), NULL);
		}
		else {
			lexer->curr_file = input;
			fseek(fd, 0L, SEEK_END);
			lexer->stream_size = ftell(fd);
			rewind(fd);
			lexer->stream_origin = newc(char, lexer->stream_size + 1);
			fread(lexer->stream_origin, sizeof(char), lexer->stream_size, fd);
			lexer->stream_origin[lexer->stream_size] = '\0';
			fclose(fd);
		}
	}
	lexer->stream = lexer->stream_origin;
	lexer->curr_line = lexer->curr_line_offset = 1;
	return lexer;
}

void lexer_free(Lexer* lexer) {
	if (lexer) {
		if (lexer->input != FROM_CHAR_PTR) {
			free(lexer->stream_origin);
		}
		free(lexer);
	}
}

Token** lex(Lexer* lexer) {
	Token** tokens = NULL;
	while (!eos(lexer)) {
		ssize_t curr_char = get_curr_char(lexer);
		if (isdigit(curr_char)) {
			sbuffer_add(tokens, get_num_token(lexer));
		}
		else if (issquote(curr_char)) {
			sbuffer_add(tokens, get_char_token(lexer));
		}
		else if (isdquote(curr_char)) {
			sbuffer_add(tokens, get_string_token(lexer));
		}
		else if (isidnt(curr_char)) {
			sbuffer_add(tokens, get_idnt_token(lexer));
		}
		else if (isknch(curr_char) >= 0) {
			sbuffer_add(tokens, get_keychar_token(lexer, isknch(curr_char)));
		}
		else {
			if (!isspace(curr_char) && !isescape(curr_char) && !issharp(curr_char)) {
				report_error(frmt("Unknown char met (code: %d): \'%c\'", (int)curr_char,
					curr_char), src_context_new(context_unpack(lexer, 1)));
			}
		}
		get_next_char(lexer);
	}
	sbuffer_add(tokens, get_eof_token(lexer));
	return tokens;
}

bool check_comment(Lexer* lexer) {
	if (!matchc(lexer, '#')) {
		return false;
	}
	// manually skip # here
	lexer->stream++;
	lexer->curr_line_offset++;
	if (*lexer->stream != '>') {
		sline_comment(lexer);
	}
	else {
		//skip ~ here
		get_next_char(lexer);
		mline_comment(lexer);
	}
	return true;
}

void sline_comment(Lexer* lexer) {
	while (!eos(lexer)) {
		get_next_char(lexer);
		if (matchc(lexer, '\n')) {
			lexer->prev_line = lexer->curr_line;
			lexer->curr_line++;
			break;
		}
	}
}

void mline_comment(Lexer* lexer) {
	while (!eos(lexer)) {
		if (!matchc(lexer, '<')) {
			get_next_char(lexer);
		}
		else {
			get_next_char(lexer);
			if (matchc(lexer, '#')) {
				lexer->curr_line_offset++;
				break;
			}
		}
	}
}

ssize_t get_next_char(Lexer* lexer) {
	if (eos(lexer)) {
		return EOF;
	}
	else {
		int32_t ch;
		switch (ch = *lexer->stream) {
			case '\r':
				lexer->prev_line_offset =
					lexer->curr_line_offset;
				lexer->curr_line_offset = 1;
				break;
			case '\n':
				lexer->prev_line = lexer->curr_line;
				lexer->curr_line++;
				break;
			case '#':
				if (!check_comment(lexer))
					goto default_case;
				break;
			default:
			default_case:
				lexer->curr_line_offset++;
				break;
		}
		return *(++lexer->stream);
	}
}

ssize_t get_curr_char(Lexer* lexer) {
	return curr_char_fits(lexer) ? *lexer->stream : EOF;
}

ssize_t unget_curr_char(Lexer* lexer) {
	if (bos(lexer)) {
		return EOF;
	}
	else {
		int32_t ch;
		lexer->stream -= 1;
		switch (ch = *lexer->stream) {
			case '\r':
				lexer->curr_line_offset =
					lexer->prev_line_offset;
				break;
			case '\n':
				lexer->curr_line =
					lexer->prev_line;
				break;
			default:
				lexer->curr_line_offset--;
				break;
		}
		return ch;
	}
}

int get_tokens_format(Lexer* lex) {
	if (matchc(lex, '0')) {
		switch (tolower(get_next_char(lex))) {
			case 'x':
				return FORMAT_HEX;
			case 'o':
				return FORMAT_OCT;
			case 'b':
				return FORMAT_BIN;
			default:
				unget_curr_char(lex);
				return FORMAT_DEC;
		}
	}
	return FORMAT_DEC;
}

char is_escape_sequence(Lexer* lex) {
	if (matchc(lex, '\\')) {
		switch (get_next_char(lex)) {
			case 'a': 	return '\a';
			case 'b': 	return '\b';
			case 'f': 	return '\f';
			case 'n': 	return '\n';
			case 'r': 	return '\r';
			case 't': 	return '\t';
			case 'v': 	return '\v';
			case '0': 	return '\0';
			case '\\': 	return '\\';
			case '\'':	return '\'';
			case '\"':	return '\"';
			default:
				unget_curr_char(lex);
				break;
		}
	}
	return -1;
}

Token* get_eof_token(Lexer* lexer) {
	Token* token = token_new(TOKEN_EOF, 
		src_context_new(context_unpack(lexer, 1)));
	return token->lexeme = "EOF", token;
}

Token* get_num_token_endian(Lexer* lexer, Token* num_token) {
	if (num_token->type == TOKEN_FLOAT_CONST) {
		return num_token;
	}
	switch (tolower(get_next_char(lexer))) {
		case 'i':
			num_token->type = TOKEN_INT_CONST;
			break;
		case 'u':
			num_token->type = TOKEN_UINT_CONST;
			break;
		case 'f':
			num_token->type = TOKEN_FLOAT_CONST;
			break;
		default:
			return unget_curr_char(lexer), num_token;
	}
	return num_token;
}

Token* get_num_token(Lexer* lexer) {
	int format = get_tokens_format(lexer);
	switch (format) {
		case FORMAT_BIN:
			return get_num_token_endian(lexer, 
				get_bin_num_token(lexer));
		case FORMAT_OCT:
			return get_num_token_endian(lexer, 
				get_oct_num_token(lexer));
		case FORMAT_HEX:
			return get_num_token_endian(lexer, 
				get_hex_num_token(lexer));
		case FORMAT_DEC:
			return get_num_token_endian(lexer,
				get_dec_num_token(lexer));
		default:
			report_error(frmt("Unknown format for number met: %d", format), NULL);
	}
	return NULL;
}

Token* get_bin_num_token(Lexer* lexer) {
	size_t size = 2;
	str_builder_create_buffer(buffer);
	str_builder_add(buffer, '0', 0);
	str_builder_add(buffer, get_curr_char(lexer), 1);

	while (isdigit_bin(get_next_char(lexer))) {
		str_builder_add(buffer, get_curr_char(lexer), size);
		size++;
	}
	unget_curr_char(lexer);
	str_builder_reduce_buffer(temp, buffer, size);
	Token* token = token_new(TOKEN_INT_CONST,
		src_context_new(context_unpack(lexer, size)));
	token->lexeme = buffer;
	return token;
}

Token* get_oct_num_token(Lexer* lexer) {
	size_t size = 2;
	str_builder_create_buffer(buffer);
	str_builder_add(buffer, '0', 0);
	str_builder_add(buffer, get_curr_char(lexer), 1);

	while (isdigit_oct(get_next_char(lexer))) {
		str_builder_add(buffer, get_curr_char(lexer), size);
		size++;
	}
	unget_curr_char(lexer);
	str_builder_reduce_buffer(temp, buffer, size);
	Token* token = token_new(TOKEN_INT_CONST,
		src_context_new(context_unpack(lexer, size)));
	token->lexeme = buffer;
	return token;
}

Token* get_hex_num_token(Lexer* lexer) {
	size_t size = 2;
	str_builder_create_buffer(buffer);
	str_builder_add(buffer, '0', 0);
	str_builder_add(buffer, get_curr_char(lexer), 1);

	while (isdigit_hex(get_next_char(lexer))) {
		str_builder_add(buffer, get_curr_char(lexer), size);
		size++;
	}
	unget_curr_char(lexer);
	str_builder_reduce_buffer(temp, buffer, size);
	Token* token = token_new(TOKEN_INT_CONST,
		src_context_new(context_unpack(lexer, size)));
	token->lexeme = buffer;
	return token;
}

Token* get_dec_num_token(Lexer* lexer) {
	size_t size = 1;
	str_builder_create_buffer(buffer);
	str_builder_add(buffer, get_curr_char(lexer), 0);

	while (isdigit_ext(get_next_char(lexer))) {
		if (matchc(lexer, '.')) {
			return get_dec_fnum_token(lexer, buffer, size);
		}
		if (matchc(lexer, 'e') || matchc(lexer, 'E')) {
			return get_dec_expnum_token(lexer, buffer, size, 0);
		}
		if (!matchc(lexer, '_')) {
			str_builder_add(buffer, get_curr_char(lexer), size), size++;
		}
	}
	unget_curr_char(lexer);
	str_builder_reduce_buffer(temp, buffer, size);
	Token* token = token_new(TOKEN_INT_CONST,
		src_context_new(context_unpack(lexer, size)));
	token->lexeme = buffer;
	return token;
}

Token* get_dec_fnum_token(Lexer* lexer, char* buffer, size_t size) {
	if (!matchc(lexer, '.')) {
		report_error(frmt("Expected \'.\' char for float number representation!"), NULL);
	}
	str_builder_add(buffer, get_curr_char(lexer), size); size++;
	get_next_char(lexer);
	if (!matchc_in(lexer, '0', '9')) {
		report_error(frmt("Expected at least one digit after \'.\'!"), 
			src_context_new(context_unpack(lexer, size)));
	}
	str_builder_add(buffer, get_curr_char(lexer), size); size++;
	while (isdigit_fext(get_next_char(lexer))) {
		if (matchc(lexer, 'e') || matchc(lexer, 'E')) {
			return get_dec_expnum_token(lexer, buffer, size, 1);
		}
		str_builder_add(buffer, get_curr_char(lexer), size);
		size++;
	}
	unget_curr_char(lexer);
	str_builder_reduce_buffer(temp, buffer, size);
	Token* token = token_new(TOKEN_FLOAT_CONST,
		src_context_new(context_unpack(lexer, size)));
	token->lexeme = buffer;
	return token;
}

Token* get_dec_expnum_token(Lexer* lexer, char* buffer, size_t size, char is_float)
{
	if (tolower(get_curr_char(lexer)) != 'e') {
		//context is null because this error in my case will never occure
		report_error(frmt("Expected exponent sign!"), NULL);
	} 
	str_builder_add(buffer, get_curr_char(lexer), size); size++;
	ssize_t ch = get_next_char(lexer);
	if (ch != '+' && ch != '-') {
		report_error(frmt("Unknown char for decimal exponent form. Expected \'-\' or \'+\'!"), 
			src_context_new(context_unpack(lexer, size)));
	}
	str_builder_add(buffer, get_curr_char(lexer), size); size++;
	ch = get_next_char(lexer);
	if (!matchc_in(lexer, '0', '9')) {
		report_error(frmt("Expected at least one digit after exponent sign!"), 
			src_context_new(context_unpack(lexer, size)));
	}
	str_builder_add(buffer, get_curr_char(lexer), size); size++;
	while (isdigit(get_next_char(lexer))) {
		str_builder_add(buffer, get_curr_char(lexer), size);
		size++;
	}
	unget_curr_char(lexer);
	str_builder_reduce_buffer(temp, buffer, size);
	Token* token = token_new(is_float ? TOKEN_FLOAT_CONST : TOKEN_INT_CONST,
		src_context_new(context_unpack(lexer, size)));
	token->lexeme = buffer;
	return token;
}

Token* get_idnt_token(Lexer* lexer) {
	size_t size = 1;
	str_builder_create_buffer(idnt);
	str_builder_add(idnt, get_curr_char(lexer), 0);

	while (isidnt_ext(get_next_char(lexer))) {
		str_builder_add(idnt, get_curr_char(lexer), size);
		size++;
	}
	unget_curr_char(lexer);
	str_builder_reduce_buffer(temp, idnt, size);
	int order = iskeyword(idnt);
	Token* token = (order >= 0) ? get_keyword_token(lexer, order) :
		token_new(TOKEN_IDNT, src_context_new(context_unpack(lexer, size)));
	token->lexeme = idnt;
	return token;
}

Token* get_char_token(Lexer* lexer)
{
	char character;
	char is_escape;
	str_builder_create_buffer(str);

	get_next_char(lexer);
	character = ((is_escape = is_escape_sequence(lexer)) >= 0) ?
		is_escape : get_curr_char(lexer);
	ssize_t curr_char = get_next_char(lexer);
	if (!matchc(lexer, '\''))
		report_error(frmt("Expected single quote, but met (code %d): \'%c\'", curr_char, curr_char),
			src_context_new(context_unpack(lexer, is_escape > 0 ? 4 : 3)));
	str_builder_add(str, character, 0);
	str_builder_reduce_buffer(temp, str, 2);
	Token* token = token_new(TOKEN_CHARACTER,
		src_context_new(context_unpack(lexer, is_escape > 0 ? 4 : 3)));
	token->lexeme = str;
	return token;
}

Token* get_string_token(Lexer* lexer) {
	size_t size = 2;
	size_t index = 0;
	ssize_t is_escape;
	ssize_t curr_char;

	str_builder_create_buffer(str);

	while (!eos(lexer) && !isdquote(get_next_char(lexer))) {
		if (iscntrl(get_curr_char(lexer))) {
			continue;
		}
		curr_char = ((is_escape = is_escape_sequence(lexer)) >= 0) ?
			is_escape : get_curr_char(lexer);
		str_builder_add(str, curr_char, index);
		index++;
		size += (is_escape > 0) ? 2 : 1;
	}
	if (!matchc(lexer, '\"')) {
		report_error(frmt("Expected double quote, but met (code %d): \'%c\'", 
			(int)curr_char, curr_char), src_context_new(context_unpack(lexer, size)));
	}
	str_builder_reduce_buffer(temp, str, index);
	Token* token = token_new(TOKEN_STRING,
		src_context_new(context_unpack(lexer, size)));
	token->lexeme = str;
	return token;
}

Token* get_keyword_token(Lexer* lexer, int order) {
	char* keyword = keywords[order];
	Token* token = token_new(order + KEYWORD_IN_TOKEN_ENUM_OFFSET,
		src_context_new(context_unpack(lexer, strlen(keyword))));
	token->lexeme = keyword;
	return token;
}

Token* get_keychar_token(Lexer* lexer, int order) {
	// if we found any equal string in ext_chars array
	bool found = true;
	size_t size = 2;
	ssize_t ret = -1;
	ssize_t type = -1;
	str_builder_create_buffer(str);
	str_builder_add(str, chars[order], 0);

	while (found && !eos(lexer)) {
		found = false;
		str_builder_add(str, get_next_char(lexer), size-1);
		str_builder_add(str, '\0', size);
		if ((ret = isextch(str)) < 0) {
			size--;
		}
		if (found = (ret >= 0)) {
			size++;
		}
		type = ret >= 0 ? ret : type;
	}
	unget_curr_char(lexer);
	str_builder_reduce_buffer(temp, str, size);
	Token* token = token_new(strlen(str) == 1 ? (order + CHARS_OFFSET) : (type + CMP_CHARS_OFFSET),
		src_context_new(context_unpack(lexer, strlen(str))));
	token->lexeme = str;
	return token;
}