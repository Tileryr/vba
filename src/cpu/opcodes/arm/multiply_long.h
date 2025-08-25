#ifndef OPCODE_MULTIPLY_LONG_INCLUDED
#define OPCODE_MULTIPLY_LONG_INCLUDED

#include "../../cpu_types.h"

typedef struct OpcodeMultiplyLong {
    OpcodeMultiplyLong(Word opcode);
    unsigned int sign : 1; // 22
    unsigned int accumulate : 1; // 21
    unsigned int set_condition_codes : 1; // 20
    unsigned int register_destination_high : 4; // 19 - 16
    unsigned int register_destination_low : 4; // 15 - 12
    unsigned int rs : 4; // 11 - 8
    unsigned int rm : 4; // 3 - 0
} MultiplyLong;


#endif