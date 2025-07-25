#ifndef ALU_INCLUDED
#define ALU_INCLUDED

#include <stdint.h>
#include "cpu_types.h"

typedef struct ArithmaticLogicUnit {
    bool carry_flag;

    Word add(int total_addends, ...);
    Word subtract(int total_subtrahends, Word base, ...);

    Word logical_right_shift(Word number, unsigned int shift_amount);
    Word logical_left_shift(Word number, unsigned int shift_amount);
    int32_t arithmetic_right_shift(int32_t number, unsigned int shift_amount);
    Word rotate_right(Word number, unsigned int rotate_amount);

    Word rotate_right_extended(Word number, bool cpsr_c);
} CpuALU;


#endif