#ifndef _ALMC_BACK_END_REGISTERS_H
#define _ALMC_BACK_END_REGISTERS_H

#include <stdio.h>
#include <stdarg.h>

#include "../../error.h"

#define reg_tostr(reg)		(reg_str[reg])
#define is_genp_reg(reg) 	(((reg) >= RAX) && ((reg) < RSP))
#define is_simd_reg(reg)	(((reg) >= XMM0) && ((reg) <= XMM15))

#define genp_regs_count 	(DIL)
#define simd_regs_count 	(XMM15-XMM0)
#define regs_count 			(simd_regs_count+genp_regs_count)

typedef uint8_t regid;

enum x86_64_registers {
	RAX, EAX, AX, AL,
	RBX, EBX, BX, BL,
	RCX, ECX, CX, CL,
	RDX, EDX, DX, DL,

	R8,  R8D,  R8W,  R8B,
	R9,  R9D,  R9W,  R9B,  
	R10, R10D, R10W, R10B,  
	R11, R11D, R11W, R11B,  
	R12, R12D, R12W, R12B,  
	R13, R13D, R13W, R13B,  
	R14, R14D, R14W, R14B,
	R15, R15D, R15W, R15B,  

	RSP, ESP, SP, SPL,
	RBP, EBP, BP, BPL,
	RSI, ESI, SI, SIL,
	RDI, EDI, DI, DIL,
}; 

enum x86_64_simd_registers {
	XMM0 = DIL+1, XMM1,  XMM2, XMM3,
	XMM4,  XMM5,  XMM6,  XMM7,
	XMM8,  XMM9,  XMM10, XMM11,
	XMM12, XMM13, XMM14, XMM15,
};

typedef enum regstate {
    REG_FREE,
    REG_RSVD,
    REG_PSHD,
} regstate;

typedef enum regtype {
	REGB = 8,		// byte, 8bit
	REGW = 16,		// word, 16bit
	REGD = 32,		// double word, 32bit
	REGQ = 64,		// quad word, 64bit
	REGV = 128		// SIMD(vector), in this case SSE, 128bit
} regtype;

static const char* reg_str[] = {
	[RAX]	= "rax",
	[EAX]	= "eax",
	[AX]	= "ax",
	[AL]	= "al",
	[RBX]	= "rbx",
	[EBX]	= "ebx",
	[BX]	= "bx",
	[BL]	= "bl",
	[RCX]	= "rcx",
	[ECX]	= "ecx",
	[CX]	= "cx",
	[CL]	= "cl",
	[RDX]	= "rdx",
	[EDX]	= "edx",
	[DX]	= "dx",
	[DL]	= "dl",
	[R8]	= "r8",
	[R8D]	= "r8d",
	[R8W]	= "r8w",
	[R8B]	= "r8b",
	[R9]	= "r9",
	[R9D]	= "r9d",
	[R9W]	= "r9w",
	[R9B]	= "r9b",
	[R10]	= "r10",
	[R10D]	= "r10d",
	[R10W]	= "r10w",
	[R10B]	= "r10b",
	[R11]	= "r11",
	[R11D]	= "r11d",
	[R11W]	= "r11w",
	[R11B]	= "r11b",
	[R12]	= "r12",
	[R12D]	= "r12d",
	[R12W]	= "r12w",
	[R12B]	= "r12b",
	[R13]	= "r13",
	[R13D]	= "r13d",
	[R13W]	= "r13w",
	[R13B]	= "r13b",
	[R14]	= "r14",
	[R14D]	= "r14d",
	[R14W]	= "r14w",
	[R14B]	= "r14b",
	[R15]	= "r15",
	[R15D]	= "r15d",
	[R15W]	= "r15w",
	[R15B]	= "r15b",
	[RSP]	= "rsp",
	[ESP]	= "esp",
	[SP]	= "sp",
	[SPL]	= "spl",
	[RBP]	= "rbp",
	[EBP]	= "ebp",
	[BP]	= "bp",
	[BPL]	= "bpl",
	[RSI]	= "rsi",
	[ESI]	= "esi",
	[SI]	= "si",
	[SIL]	= "sil",
	[RDI]	= "rdi",
	[EDI]	= "edi",
	[DI]	= "di",
	[DIL]	= "dil",
	[XMM0]	= "xmm0",
	[XMM1]	= "xmm1",
	[XMM2]	= "xmm2",
	[XMM3]	= "xmm3",
	[XMM4]	= "xmm4",
	[XMM5]	= "xmm5",
	[XMM6]	= "xmm6",
	[XMM7]	= "xmm7",
	[XMM8]	= "xmm8",
	[XMM9]	= "xmm9",
	[XMM10]	= "xmm10",
	[XMM11]	= "xmm11",
	[XMM12]	= "xmm12",
	[XMM13]	= "xmm13",
	[XMM14]	= "xmm14",
	[XMM15]	= "xmm15"
};

void reserve(regid reg);
regstate unreserve(regid reg);
regid reserve_any(regtype type);

#endif