#ifndef OPCODE_SINGLE_DATA_TRANSFER_INCLUDED
#define OPCODE_SINGLE_DATA_TRANSFER_INCLUDED

#include "../cpu_types.h"
#include "./data_transfer.h"
typedef struct OpcodeSingleDataTransfer : DataTransfer {
    OpcodeSingleDataTransfer(Word opcode);
    unsigned int i : 1; // Immediate/Register : 25

    unsigned int b : 1; // Byte/Word : 22
    
    // if (i == 0)
    unsigned int offset_immediate : 12; // 11-0
    // if (i == 1) 
    unsigned int register_shift_amount : 5; // 11 - 7
    unsigned int register_shift_type : 2; // 6 - 5

    void static load(ARM7TDMI * cpu, Word address, Byte destination_register, bool byte);
    void static load(ARM7TDMI * cpu, Word address, Word value, Byte destination_register, bool byte);
    void static store(ARM7TDMI * cpu, Word address, Byte source_register, bool byte);
} OpcodeSingleDataTransfer;

#endif