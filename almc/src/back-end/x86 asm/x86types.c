#include "x86types.h"

static const char* predefined_types_prefixes[] = {
	"BYTE",
	"WORD",
	"DWORD",
	"QWORD",
	"REAL4",
	"REAL8",
};

int get_type_prefix(Type* type)
{
	if (type->spec.ptr_rank)
		return DWORD;
	if (!type->spec.is_predefined)
		assert(0);
	else
	{
		if (strcmp(type->repr, "i8") == 0 ||
			strcmp(type->repr, "u8") == 0 ||
			strcmp(type->repr, "chr") == 0)
			return BYTE;
		else if (strcmp(type->repr, "i16") == 0 ||
			strcmp(type->repr, "u16") == 0)
			return WORD;
		else if (strcmp(type->repr, "i32") == 0 ||
			strcmp(type->repr, "u32") == 0 ||
			strcmp(type->repr, "f32") == 0 ||
			strcmp(type->repr, "str") == 0)
			return DWORD;
		else if (strcmp(type->repr, "i64") == 0 ||
			strcmp(type->repr, "u64") == 0 ||
			strcmp(type->repr, "f64") == 0)
			return QWORD;
		else
			assert(0);
	}
}

const char* get_predefined_type_str(int prefix)
{
	assert(prefix >= BYTE && prefix <= REAL8);
	return predefined_types_prefixes[prefix];
}

int get_type_size2(Type* type)
{
	assert(0);
	if (type->spec.ptr_rank)
		return 4;
	if (!type->spec.is_predefined)
		assert(0);
	else
	{
		switch (get_type_prefix(type))
		{
		case BYTE:
			return 1;
		case WORD:
			return 2;
		case DWORD:
		case REAL4:
			return 4;
		case QWORD:
		case REAL8:
			return 8;
		}
	}
	assert(0);
	return -1;
}