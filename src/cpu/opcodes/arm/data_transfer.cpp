#include "data_transfer.h"
#include "src/cpu/cpu.h"

DataTransfer::DataTransfer() {}
DataTransfer::DataTransfer(Word opcode) {
    p = Utils::read_bit(opcode, 24);
    u = Utils::read_bit(opcode, 23);

    w = Utils::read_bit(opcode, 21);
    l = Utils::read_bit(opcode, 20);

    base_register = Utils::read_bit_range(opcode, 16, 19);
    source_destination_register = Utils::read_bit_range(opcode, 12, 15);
    
    offset_register = Utils::read_bit_range(opcode, 0, 3);
}

Word DataTransfer::calculate_address(ARM7TDMI * cpu, Word offset) {
    Word address;

    Word offseted_base_register;
    Word base_register_value = cpu->read_register(base_register);

    if (base_register == REGISTER_PC) {
        if (w) {
            cpu->warn("Single Data Transfer - Base register == PC && writeback");
        }
        if (cpu->cpsr.t == STATE_ARM) {
            base_register_value += 8;
        } else {
            base_register_value = (base_register_value + 4) & (~0b11);
        }
    }

    if (u == 0) { // Down - Subtract
        offseted_base_register = base_register_value - offset;
    } else { // Up - Add
        offseted_base_register = base_register_value + offset;
    }

    if (p == 1) { // Pre-Indexed
        address = offseted_base_register;
    } else {
        address = base_register_value;
        cpu->write_register(
            base_register, 
            offseted_base_register
        );
    }

    if (w == 1) { // Write-back
        cpu->write_register(
            base_register, 
            address
        );
    }

    return address;
}