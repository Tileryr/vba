#ifndef OPCODE_BLOCK_DATA_TRANSFER_INCLUDED
#define OPCODE_BLOCK_DATA_TRANSFER_INCLUDED

#include "../cpu_types.h"

typedef struct OpcodeBlockDataTransfer
{
    OpcodeBlockDataTransfer(Word opcode);
    unsigned int p : 1; // 24
    unsigned int u : 1; // 23
    unsigned int s : 1; // 22
    unsigned int w : 1; // 21
    unsigned int l : 1; // 20

    unsigned int base_register : 4; // 16 - 19
    unsigned int register_list : 16; // 0 - 15
} OpcodeBlockDataTransfer;

#endif