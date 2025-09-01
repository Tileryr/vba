#include "half_word_signed_data_transfer.h"
#include "src/cpu/alu.h"
#include "src/utils.h"

OpcodeHalfWordSignedDataTransfer::OpcodeHalfWordSignedDataTransfer() {}
OpcodeHalfWordSignedDataTransfer::OpcodeHalfWordSignedDataTransfer(Word opcode) : DataTransfer(opcode) {
    i = Utils::read_bit(opcode, 22);
    s = Utils::read_bit(opcode, 6);
    h = Utils::read_bit(opcode, 5);
    immediate_high_nibble = Utils::read_bit_range(opcode, 8, 11);
    immediate_low_nibble = Utils::read_bit_range(opcode, 0, 3);
}

void OpcodeHalfWordSignedDataTransfer::run(ARM7TDMI * cpu) {
    Word offset;

    if (offset_register == REGISTER_PC) {
        cpu->warn("Single Data Transfer - Offset register == PC");
    }

    if (i == 0) { // Register Offset
        offset = cpu->read_register(offset_register);
    } else { // Immediate Offset
        offset = (immediate_high_nibble << 4) | immediate_low_nibble;
    }

    Word source_register_value = cpu->read_register(source_destination_register);
    if (source_destination_register == REGISTER_PC) {
        source_register_value += 12;
    }

    Word address = calculate_address(cpu, offset);
    Word aligned_address = address & (~0b1);

    // For little endian
    if (l == 0) { // Store
        if (s == 0) { // Unsigned
            if (h == 0) { // Byte || Reserved for SWP

            } else { // Halfword || STRH
                HalfWord selected_halfword = source_register_value & 0xFFFF;
                cpu->memory.write_halfword_to_memory(aligned_address, selected_halfword);
            }
        } else { // Signed
            cpu->warn("Half-Word signed data transfer - Storing signed value");
        }
    } else { // Load
        Word selected_halfword = cpu->memory.read_halfword_from_memory(aligned_address);
        Byte rotate_amount = (address & 1) * 8;
        selected_halfword = CpuALU().rotate_right(selected_halfword, rotate_amount);

        if (s == 0) { // Unsigned
            if (h == 0) { // Byte || Reserved for SWP

            } else { // Halfword || LDRH
                cpu->write_register(source_destination_register, selected_halfword);
            }
        } else { // Signed
            if (h == 0) { // Byte || LDRSB
                Byte selected_byte = cpu->memory.read_from_memory(address);
                cpu->write_register(source_destination_register, Utils::sign_extend(selected_byte, 8));
            } else { // Halfword || LDRSH
                cpu->write_register(source_destination_register, Utils::sign_extend(selected_halfword, 16 - rotate_amount));
            }
        }
    }
}

OpcodeHalfWordSignedDataTransferBuilder::OpcodeHalfWordSignedDataTransferBuilder(bool load, Word base_register, Word source_destination_register) {
    product.l = load;
    product.base_register = base_register;
    product.source_destination_register = source_destination_register;
}

OpcodeHalfWordSignedDataTransferBuilder& OpcodeHalfWordSignedDataTransferBuilder::set_datatype(OpcodeHalfWordSignedDataTransfer::DataType opcode) {
    product.h = Utils::read_bit(opcode, 0);
    product.s = Utils::read_bit(opcode, 1);
    return *this;
}
OpcodeHalfWordSignedDataTransferBuilder& OpcodeHalfWordSignedDataTransferBuilder::set_flags(bool pre, bool up, bool writeback) {
    product.p = pre;
    product.u = up;
    product.w = writeback;
    return *this;
}
OpcodeHalfWordSignedDataTransferBuilder& OpcodeHalfWordSignedDataTransferBuilder::set_offset_immediate(Byte immediate) {
    product.i = true;
    product.immediate_high_nibble = (immediate >> 4) & 0xF;
    product.immediate_low_nibble = immediate & 0xF;
    return *this;
}
OpcodeHalfWordSignedDataTransferBuilder& OpcodeHalfWordSignedDataTransferBuilder::set_offset_register(Byte offset_register) {
    product.i = false;
    product.offset_register = offset_register;
    return *this;
}

OpcodeHalfWordSignedDataTransfer OpcodeHalfWordSignedDataTransferBuilder::get_product() {
    return product;
}