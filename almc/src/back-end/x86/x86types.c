#include "x86types.h"

static const char* predefined_types_prefixes[] = {
	"BYTE",
	"WORD",
	"DWORD",
	"QWORD",
	"REAL4",
	"REAL8",
};

char* get_ptr_prefix(Type* type)
{
	if (IS_STRUCT_OR_UNION_TYPE(type))
		return "dword";
	else
	{
		if (IS_ARRAY_TYPE(type))
			type = type->base;
		switch (type->size)
		{
		case I8_SIZE:
			return "byte";
		case I16_SIZE:
			return "word";
		case I32_SIZE:
			return is_real_type(type) ?
				"real4" : "dword";
		}
	}
	assert(0);
}