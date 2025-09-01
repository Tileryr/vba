#ifndef OPCODE_SINGLE_DATA_TRANSFER_INCLUDED
#define OPCODE_SINGLE_DATA_TRANSFER_INCLUDED

#include "src/cpu/cpu_types.h"
#include "./data_transfer.h"

typedef struct OpcodeSingleDataTransfer : DataTransfer {
    OpcodeSingleDataTransfer();
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
    void static store(ARM7TDMI * cpu, Word address, Word source_register_value, bool byte);

    void run(ARM7TDMI * cpu);
} OpcodeSingleDataTransfer;

typedef struct OpcodeSingleDataTransferBuilder {
    OpcodeSingleDataTransferBuilder(bool load, Word base_register, Word source_destination_register);
    OpcodeSingleDataTransfer product;

    OpcodeSingleDataTransferBuilder& set_flags(bool pre, bool up, bool byte, bool writeback);
    OpcodeSingleDataTransferBuilder& set_offset_immediate(Word immediate);
    OpcodeSingleDataTransferBuilder& set_offset_register(Byte offset_register, Byte shift_amount, Byte shift_type);

    OpcodeSingleDataTransfer get_product();
} OpcodeSingleDataTransferBuilder;

#endif