#ifndef OPCODE_SINGLE_DATA_TRANSFER_INCLUDED
#define OPCODE_SINGLE_DATA_TRANSFER_INCLUDED

#include "../cpu_types.h"

typedef struct OpcodeSingleDataTransfer {
    OpcodeSingleDataTransfer(Word opcode);
    unsigned int i : 1; // Immediate/Register : 25
    unsigned int p : 1; // Pre/Post : 24
    unsigned int u : 1; // Up/Down : 23
    unsigned int b : 1; // Byte/Word : 22
    unsigned int w : 1; // Write-back : 21
    unsigned int l : 1; // Load/Store : 20

    unsigned int source_destination_register : 4; // 19-16
    unsigned int base_register : 4; // 15-12
    
    // if (i == 0)
    unsigned int offset_immediate : 12; // 11-0
    // if (i == 1) 
    unsigned int register_shift_amount : 5; // 11 - 7
    unsigned int register_shift_type : 2; // 6 - 5
    // 0; // 4
    unsigned int offset_register : 4; // 3 - 0
} OpcodeSingleDataTransfer;

#endif