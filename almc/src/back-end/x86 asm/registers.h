#ifndef ALMC_BACK_END_x86_REGISTERS
#define ALMC_BACK_END_x86_REGISTERS

#define EAX 0
#define EBX 1
#define ECX 2
#define EDX 3
#define ESP 4
#define EBP 5
#define ESI 6

/*#define AX 8 
#define BX 9
#define CX 10
#define DX 11
#define SP 12
#define BP 13
#define SI 14

#define	AH 15
#define	AL 16

#define	BH 17
#define	BL 18

#define	CH 19
#define	CL 20

#define	DH 21
#define	DL 22

#define	AH 23
#define	AL 24

#define	AH 25
#define	AL 26

#define	AH 27
#define	AL 28*/

#define REGISTER_FREE		   0xC
#define REGISTER_RESERVED	   0xD
#define REGISTER_NEEDS_RESTORE 0xF

//#define REGISTER_SIZE(reg) \
//	(reg >= EAX && reg <= ESI) ? 32 

#define IS_GENERAL_PURPOSE_REGISTER(reg) \ 

typedef enum x86_RegistersSize
{
	BITS32,
	BITS16,
	BITS8
} RegistersSize;

typedef struct x86_RegisterTable
{
	//todo: extend for minors
	char ret_reg;
	char reg_table[6] = { 0 };
} RegisterTable;

int reserve_gp_register(RegistersSize size);
int unreserve_register(int reg);

#endif