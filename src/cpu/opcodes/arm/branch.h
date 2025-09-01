#include "src/cpu/cpu.h"
#include "src/cpu/cpu_types.h"

typedef struct OpcodeBranch {
    OpcodeBranch(Word opcode);

    unsigned int link_bit : 1; // 24
    unsigned int offset : 24; // 23 - 0
    
    static void branch(ARM7TDMI * cpu, Word pc, Word offset_immediate, Byte bits_in_offset);
} OpcodeBranch;