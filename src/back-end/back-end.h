#ifndef _ALMC_BACK_END_H
#define _ALMC_BACK_END_H
    #ifdef _ALMC_x86_64
        #include "x86_64/ir.h"
        #include "x86_64/gen.h"
        #include "x86_64/registers.h"
        #include "x86_64/instructions.h"
    #else
        #error "Current architecture is not supported."
    #endif // _ALMC_x86_64
#endif // _ALMC_BACK_END_H