#ifndef OPCODE_SWAP_INCLUDED
#define OPCODE_SWAP_INCLUDED

#include "src/cpu/cpu_types.h"
typedef struct OpcodeSwap {
    OpcodeSwap(Word opcode);
    unsigned int b : 1; // 22
    unsigned int base_register : 4; // 16 - 19
    unsigned int destination_register : 4; // 12 - 15
    unsigned int source_register : 4; // 0 - 3
} OpcodeSwap;

#endif