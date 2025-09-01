#ifndef OPCODE_HALF_WORD_SIGNED_DATA_TRANSFER_INCLUDED
#define OPCODE_HALF_WORD_SIGNED_DATA_TRANSFER_INCLUDED

#include "src/cpu/cpu_types.h"
#include "./data_transfer.h"

typedef struct OpcodeHalfWordSignedDataTransfer : DataTransfer {
    enum DataType {
        UNSIGNED_HALFWORD = 1,
        SIGNED_BYTE = 2,
        SIGNED_HALFWORD = 3
    };

    OpcodeHalfWordSignedDataTransfer();
    OpcodeHalfWordSignedDataTransfer(Word opcode);
    
    unsigned int i : 1; // Immediate/Register offset (1 = immediate, 0 = register) : 22

    unsigned int s : 1; // 6
    unsigned int h : 1; // 5

    // if i = 0 (register offset)
    

    // if i = 1 (immediate offset)

    unsigned int immediate_high_nibble : 4; // 8 - 11
    unsigned int immediate_low_nibble : 4; // 0 - 3

    void run(ARM7TDMI * cpu);
} OpcodeHalfWordSignedDataTransfer;

typedef struct OpcodeHalfWordSignedDataTransferBuilder {
    OpcodeHalfWordSignedDataTransferBuilder(bool load, Word base_register, Word source_destination_register);
    OpcodeHalfWordSignedDataTransfer product;

    OpcodeHalfWordSignedDataTransferBuilder& set_datatype(OpcodeHalfWordSignedDataTransfer::DataType opcode);
    OpcodeHalfWordSignedDataTransferBuilder& set_flags(bool pre, bool up, bool writeback);
    OpcodeHalfWordSignedDataTransferBuilder& set_offset_immediate(Byte immediate);
    OpcodeHalfWordSignedDataTransferBuilder& set_offset_register(Byte offset_register);

    OpcodeHalfWordSignedDataTransfer get_product();
} OpcodeHalfWordSignedDataTransferBuilder;

#endif