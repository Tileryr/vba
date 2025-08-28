#ifndef OPCODE_BRANCH_EXCHANGE_INCLUDED
#define OPCODE_BRANCH_EXCHANGE_INCLUDED

#include "src/cpu/cpu.h"
#include "src/cpu/cpu_types.h"

typedef struct OpcodeBranchExchange
{
    OpcodeBranchExchange();
    OpcodeBranchExchange(Word opcode);

    unsigned int register_number : 4;

    static void build(OpcodeBranchExchange * target, Word register_number);

    void run(ARM7TDMI * cpu);
} OpcodeBranchExchange;

#endif