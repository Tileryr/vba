#include "single_data_transfer.h"
#include "../../utils.h"
#include "../cpu.h"
#include "./data_processing.h"
#include "../alu.h"

OpcodeSingleDataTransfer::OpcodeSingleDataTransfer(Word opcode) : DataTransfer(opcode) {
    i = Utils::read_bit(opcode, 25);


    b = Utils::read_bit(opcode, 22);

    
    offset_immediate = Utils::read_bit_range(opcode, 0, 11);
    register_shift_amount = Utils::read_bit_range(opcode, 7, 11);
    register_shift_type = Utils::read_bit_range(opcode, 5, 6);
}

void OpcodeSingleDataTransfer::load(ARM7TDMI * cpu, Word address, Byte destination_register, bool byte) { // Memory -> Register
    if (byte == 1) { // Byte
        cpu->write_register(
        destination_register,
        cpu->memory[address]
        );
    } else { // Word
        Word aligned_address = address & (~0b11);
        Word word_at_address = cpu->read_word_from_memory(aligned_address);
        Word rotated_word_at_address = CpuALU().rotate_right(word_at_address, (address & 3) * 8); 
        cpu->write_register(
            destination_register,
            rotated_word_at_address
        );
    }
}

void OpcodeSingleDataTransfer::load(ARM7TDMI * cpu, Word address, Word value, Byte destination_register, bool byte) {
    if (byte == 1) { // Byte
        cpu->write_register(
        destination_register,
        value & 0xFF
        );
    } else { // Word
        Word aligned_address = address & (~0b11);
        Word rotated_word_at_address = CpuALU().rotate_right(value, (address & 3) * 8); // Rotate it so that the first byte is the target byte in the address. 
        cpu->write_register(
            destination_register,
            rotated_word_at_address
        );
    }
}

void OpcodeSingleDataTransfer::store(ARM7TDMI * cpu, Word address, Byte source_register, bool byte) { // Register -> Memory
    Word source_register_value = cpu->read_register(source_register);
    if (source_register == REGISTER_PC) {
        source_register_value += 12;
    }

    if (byte == 1) { // Byte
        Byte stored_byte = source_register_value & 0xFF;
        cpu->memory[address] = stored_byte;
    } else { // Word
        Word aligned_address = address & (~0b11);
        cpu->write_word_to_memory(aligned_address, source_register_value);
    }
}