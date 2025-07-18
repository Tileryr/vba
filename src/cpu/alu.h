#ifndef ALU_INCLUDED
#define ALU_INCLUDED

#include "psr.h"

typedef struct ArithmaticLogicUnit {
    bool carry_flag;

    Word add(int total_addends, ...);
    Word subtract(int total_subtrahends, Word base, ...);
} CPUMATH;


#endif