#ifndef OPCODE_PSR_TRANSFER_INCLUDED
#define OPCODE_PSR_TRANSFER_INCLUDED

#include "../cpu_types.h"

typedef struct OpcodePsrTransfer {
    OpcodePsrTransfer(Word opcode);
    enum TransferType {
        MRS,
        MSR
    };

    unsigned int psr : 1; // 22
    unsigned int is_msr_instruction : 1; // 21

    // MRS
    unsigned int register_destination : 4; // 12 - 15

    // MSR
    unsigned int only_write_flag : 1; // 19
    unsigned int register_source : 4; // 0 - 3

    // If only only_write_flag
        unsigned int immediate_operand : 1; // 25
        // If immediate_operand
            unsigned int immediate_value : 8; // 0 - 7
            unsigned int immediate_rotate : 4; // 8 - 11
    
} OpcodePsrTransfer;

#endif