#include "single_data_transfer.h"
#include "src/utils.h"
#include "src/cpu/cpu.h"
#include "src/cpu/opcodes/arm/data_processing.h"
#include "src/cpu/alu.h"

OpcodeSingleDataTransfer::OpcodeSingleDataTransfer() {}
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
        cpu->memory.read_from_memory(address)
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

void OpcodeSingleDataTransfer::store(ARM7TDMI * cpu, Word address, Word source_register_value, bool byte) { // Register -> Memory
    if (byte == 1) { // Byte
        Byte stored_byte = source_register_value & 0xFF;
        cpu->memory.write_to_memory(address, stored_byte);
    } else { // Word
        Word aligned_address = address & (~0b11);
        cpu->write_word_to_memory(aligned_address, source_register_value);
    }
}

void OpcodeSingleDataTransfer::run(ARM7TDMI * cpu) {
    if (offset_register == REGISTER_PC) {
        cpu->warn("Single Data Transfer - Offset register == PC");
    }

    Word offset;

    if (i == 1) { // Offset = Shifted Register
        Word offset_register_value = cpu->read_register(offset_register);
        OpcodeDataProcess::BitShiftType shift_type = static_cast<OpcodeDataProcess::BitShiftType>(register_shift_type);
        CpuALU alu;
        offset = OpcodeDataProcess::shift_op2(&alu, offset_register_value, register_shift_amount, shift_type, cpu->cpsr.c);
    } else { // Offset = immediate value
        offset = offset_immediate;
    }

    Word source_destination_register_value = cpu->read_register(source_destination_register);
    Word address = calculate_address(cpu, offset);

    // For little endian only
    if (l == 0) { // Store
        if (source_destination_register == REGISTER_PC) {
            source_destination_register_value += 12;
        }
        store(cpu, address, source_destination_register_value, b);
    } else { // Load
        load(cpu, address, source_destination_register, b);
    }
}

OpcodeSingleDataTransferBuilder::OpcodeSingleDataTransferBuilder(bool load, Word base_register, Word source_destination_register) {
    product.l = load;
    product.base_register = base_register;
    product.source_destination_register = source_destination_register;
}

OpcodeSingleDataTransferBuilder& OpcodeSingleDataTransferBuilder::set_flags(bool pre, bool up, bool byte, bool writeback) {
    product.p = pre;
    product.u = up;
    product.b = byte;
    product.w = writeback;
    return *this;
}
OpcodeSingleDataTransferBuilder& OpcodeSingleDataTransferBuilder::set_offset_immediate(Word immediate) {
    product.i = false;
    product.offset_immediate = immediate;
    return *this;
}
OpcodeSingleDataTransferBuilder& OpcodeSingleDataTransferBuilder::set_offset_register(Byte offset_register, Byte shift_amount, Byte shift_type) {
    product.i = true;
    product.offset_register = offset_register;
    product.register_shift_amount = shift_amount;
    product.register_shift_type = shift_type;
    return *this;
}

OpcodeSingleDataTransfer OpcodeSingleDataTransferBuilder::get_product() {
    return product;
}