typedef enum x86_AsmCommandKind
{
	EMPTY_COMMAND,
	UNARY_COMMAND,
	BINARY_COMMAND,
} AsmCommandKind;

/*
	register
	memory
	immediate
*/

typedef enum x86_AsmArgumentKind
{
	ARG_MEMORY,
	ARG_REGISTER,
	ARG_IMMEDIATE,
} AsmArgumentKind;

typedef struct x86_AsmCommandArgument
{
	int size;
	char* value;
	AsmArgumentKind kind;
} AsmCommandArgument;

typedef struct x86_AsmCodeLine
{
	char* command;
	char** arguments;
	AsmCommandKind kind;
} AsmCodeLine;

typedef enum x86_AsmDataKind
{
	DATA_ARRAY,
	DATA_VARIABLE
} AsmDataKind;

typedef struct x86_AsmDataLine
{
	//-----------
	// in case of an array (dup)
	int count;    
	char** elements;
	//-----------
	char* prefix;
	AsmDataKind kind;
} AsmDataLine;

typedef struct x86_AsmCodeSegment
{

} AsmCodeSegment;

typedef struct x86_AsmDataSegment
{

} AsmDataSegment;

typedef struct x86_AsmProgram
{
	char* output;
	char** includes;
	AsmCodeSegment* code_seg;
	AsmDataSegment* data_seg;
} AsmProgram;