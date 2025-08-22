#ifndef OPCODE_HALF_WORD_SIGNED_DATA_TRANSFER_INCLUDED
#define OPCODE_HALF_WORD_SIGNED_DATA_TRANSFER_INCLUDED

#include "../cpu_types.h"
#include "./data_transfer.h"
typedef struct OpcodeHalfWordSignedDataTransfer : DataTransfer {
    OpcodeHalfWordSignedDataTransfer(Word opcode);
    
    unsigned int i : 1; // Immediate/Register offset (1 = immediate, 0 = register) : 22

    unsigned int s : 1; // 6
    unsigned int h : 1; // 5

    // if i = 0 (register offset)
    

    // if i = 1 (immediate offset)

    unsigned int immediate_high_nibble : 4; // 8 - 11
    unsigned int immediate_low_nibble : 4; // 0 - 3
    
} OpcodeHalfWordSignedDataTransfer;

#endif