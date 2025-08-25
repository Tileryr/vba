#ifndef OPCODE_MULTIPLY_INCLUDED
#define OPCODE_MULTIPLY_INCLUDED

#include "src/cpu/cpu_types.h"

typedef struct OpcodeMultiply {
    OpcodeMultiply(Word opcode);
    unsigned int accumulate : 1; // 21
    unsigned int set_condition_codes : 1; // 20
    unsigned int register_destination : 4; // 19 - 16
    unsigned int rn : 4; // 15 - 12
    unsigned int rs : 4; // 11 - 8
    unsigned int rm : 4; // 3 - 0
} Multiply;


#endif