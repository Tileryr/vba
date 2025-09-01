#ifndef OPCODE_BLOCK_DATA_TRANSFER_INCLUDED
#define OPCODE_BLOCK_DATA_TRANSFER_INCLUDED

#include "src/cpu/cpu.h"
#include "src/cpu/cpu_types.h"

typedef struct OpcodeBlockDataTransfer
{
    OpcodeBlockDataTransfer();
    OpcodeBlockDataTransfer(Word opcode);
    unsigned int p : 1; // 24
    unsigned int u : 1; // 23
    unsigned int s : 1; // 22
    unsigned int w : 1; // 21
    unsigned int l : 1; // 20

    unsigned int base_register : 4; // 16 - 19
    unsigned int register_list : 16; // 0 - 15

    void run(ARM7TDMI * cpu);
} OpcodeBlockDataTransfer;

typedef struct OpcodeBlockDataTransferBuilder {
    OpcodeBlockDataTransferBuilder(bool load, Byte base_register);
    OpcodeBlockDataTransfer product;

    OpcodeBlockDataTransferBuilder& set_flags(bool pre, bool up, bool s_flag, bool writeback);
    OpcodeBlockDataTransferBuilder& set_register_list(HalfWord register_list);

    OpcodeBlockDataTransfer get_product();
} OpcodeBlockDataTransferBuilder;


#endif