#include <cstdio>
#include <limits.h>
#include <stdlib.h>

#include "../cpu_types.h"
#include "../alu.h"
#include "../cpu.h"

#include "opcode_types.h"

#include "./arm/data_processing.h"
#include "./arm/branch_exchange.h"
#include "./arm/multiply.h"
#include "./arm/multiply_long.h"
#include "./arm/psr_transfer.h"
#include "./arm/single_data_transfer.h"
#include "./arm/half_word_signed_data_transfer.h"
#include "./arm/block_data_transfer.h"
#include "./arm/swap.h"

void ARM7TDMI::arm_opcode_branch(Word opcode)
{
    Word pc_with_prefetch_offset = read_register(REGISTER_PC) + 8; // Other 4 is accounted for after when incrementing instruction
    int32_t offset = Utils::read_bit_range(opcode, 0, 23);
    bool link_bit = Utils::read_bit(opcode, 24);

    offset = offset << 2;
    offset = Utils::sign_extend(offset, 26);

    if (link_bit)
    {
        Word return_address = pc_with_prefetch_offset - 4;
        write_register(REGISTER_LR, return_address);
    }

    Word new_address = pc_with_prefetch_offset + offset;
    write_register(REGISTER_PC, new_address);
}

void ARM7TDMI::arm_opcode_branch_exchange(Word opcode)
{
    OpcodeBranchExchange(opcode).run(this);
}

void ARM7TDMI::arm_opcode_software_interrupt(Word opcode)
{
    Word comment_field = Utils::read_bit_range(opcode, 0, 23);
    switch (comment_field)
    {
        case 0x60000: { // DIVISION
            int32_t number = read_register(0);
            int32_t denom = read_register(1);

            write_register(0, number/denom);
            write_register(1, number%denom);
            write_register(3, abs(number/denom));
            break;
        }
        default:
            run_exception(EXCEPTION_SOFTWARE_INTERRUPT);
            break;
    }
    
}

void ARM7TDMI::arm_opcode_undefined_intruction(Word opcode)
{
    run_exception(EXCEPTION_UNDEFINED);
}

void ARM7TDMI::arm_opcode_data_processing(Word opcode) 
{   
    OpcodeDataProcess opcode_class = OpcodeDataProcess(opcode);
    opcode_class.run(this);
}

void ARM7TDMI::arm_opcode_multiply(Word opcode)
{
    OpcodeMultiply multiply = OpcodeMultiply(opcode);
    multiply.run(this);
};

void ARM7TDMI::arm_opcode_multiply_long(Word opcode)
{
    OpcodeMultiplyLong multiply_long = OpcodeMultiplyLong(opcode);
    u_int64_t result;
    
    // int32_t rs_value_s = rs_value_u;
    // int32_t rm_value_s = rs_value_u;
    
    if (multiply_long.sign == 0) { // Unsigned
        u_int64_t rs_value_u = read_register(multiply_long.rs);
        u_int64_t rm_value_u = read_register(multiply_long.rm);
        result = rm_value_u * rs_value_u;

        if (multiply_long.accumulate) {
            u_int64_t rdhi = read_register(multiply_long.register_destination_high);
            u_int64_t rdlo = read_register(multiply_long.register_destination_low);
            u_int64_t rdhilo = (rdhi << 32) | (rdlo);
            result += rdhilo;
        }
    } else { // Signed
        int64_t result_signed;
        int32_t rs_value_u = read_register(multiply_long.rs);
        int32_t rm_value_u = read_register(multiply_long.rm);
        result_signed = rm_value_u * rs_value_u;

        if (multiply_long.accumulate) {
            int64_t rdhi = read_register(multiply_long.register_destination_high);
            int64_t rdlo = read_register(multiply_long.register_destination_low);
            int64_t rdhilo = (rdhi << 32) | (rdlo & UINT32_MAX);
            result_signed += rdhilo;
        }

        result = result_signed;
    } 

    Word destination_high_value = result >> 32;
    Word destination_low_value = result & UINT32_MAX;

    write_register(multiply_long.register_destination_high, destination_high_value);
    write_register(multiply_long.register_destination_low, destination_low_value);

    if (multiply_long.set_condition_codes)
    {
        cpsr.n = Utils::read_bit(destination_high_value, 31);
        cpsr.z = result == 0;
        cpsr.c = rand() & 1;
        cpsr.v = rand() & 1;
    }
}
 
void ARM7TDMI::arm_opcode_psr_transfer(Word opcode)
{
    OpcodePsrTransfer psr_transfer = OpcodePsrTransfer(opcode);
    PSR * target_psr = psr_transfer.psr == 0 
        ? &cpsr 
        : &current_register_set()->spsr;

    if (psr_transfer.is_msr_instruction) {
        Word write_value;
        if (psr_transfer.immediate_operand) {
            CpuALU alu;
            Byte immediate = psr_transfer.immediate_value;
            Byte immediate_rotate = psr_transfer.immediate_rotate * 2;
            write_value = alu.rotate_right(immediate, immediate_rotate);
        } else {
            write_value = read_register(psr_transfer.register_source);
        }
        Word new_psr_value = target_psr->value();

        if (psr_transfer.write_flag_bits) {
            Byte new_flag_bits = Utils::read_bit_range(write_value, 24, 31);
            Utils::write_bit_range(&new_psr_value, 24, 31, new_flag_bits);
        }
        if (psr_transfer.write_control_bits) {
            Byte new_control_bits = Utils::read_bit_range(write_value, 0, 7);
            Utils::write_bit_range(&new_psr_value, 0, 7, new_control_bits);
        }

        target_psr->write_value(new_psr_value);
    } else { // MRS
        Byte destination_register = psr_transfer.register_destination;
        write_register(destination_register, target_psr->value());  
    }
};

void ARM7TDMI::arm_opcode_single_data_transfer(Word opcode)
{
    OpcodeSingleDataTransfer data_transfer = OpcodeSingleDataTransfer(opcode);
    
    if (data_transfer.offset_register == REGISTER_PC) {
        warn("Single Data Transfer - Offset register == PC");
    }

    Word offset;

    if (data_transfer.i == 1) { // Offset = Shifted Register
        Word offset_register_value = read_register(data_transfer.offset_register);
        OpcodeDataProcess::BitShiftType shift_type = static_cast<OpcodeDataProcess::BitShiftType>(data_transfer.register_shift_type);
        CpuALU alu;
        offset = OpcodeDataProcess::shift_op2(&alu, offset_register_value, data_transfer.register_shift_amount, shift_type, cpsr.c);
    } else { // Offset = immediate value
        offset = data_transfer.offset_immediate;
    }

    Word source_destination_register_value = read_register(data_transfer.source_destination_register);
    Word address = data_transfer.calculate_address(this, offset);

    // For little endian only
    if (data_transfer.l == 0) { // Store
        if (data_transfer.source_destination_register == REGISTER_PC) {
            source_destination_register_value += 12;
        }
        data_transfer.store(this, address, source_destination_register_value, data_transfer.b);
    } else { // Load
        data_transfer.load(this, address, data_transfer.source_destination_register, data_transfer.b);
    }
}

void ARM7TDMI::arm_opcode_half_word_signed_data_transfer(Word opcode) {
    OpcodeHalfWordSignedDataTransfer data_transfer = OpcodeHalfWordSignedDataTransfer(opcode);

    Word offset;

    if (data_transfer.offset_register == REGISTER_PC) {
        warn("Single Data Transfer - Offset register == PC");
    }

    if (data_transfer.i == 0) { // Register Offset
        offset = read_register(data_transfer.offset_register);
    } else { // Immediate Offset
        offset = (data_transfer.immediate_high_nibble << 4) | data_transfer.immediate_low_nibble;
    }

    Word source_register_value = read_register(data_transfer.source_destination_register);
    if (data_transfer.source_destination_register == REGISTER_PC) {
        source_register_value += 12;
    }

    Word address = data_transfer.calculate_address(this, offset);
    Word aligned_address = address & (~0b1);

    // For little endian
    if (data_transfer.l == 0) { // Store
        if (data_transfer.s == 0) { // Unsigned
            if (data_transfer.h == 0) { // Byte || Reserved for SWP

            } else { // Halfword || STRH
                HalfWord selected_halfword = source_register_value & 0xFFFF;
                memory.write_halfword_to_memory(aligned_address, selected_halfword);
            }
        } else { // Signed
            warn("Half-Word signed data transfer - Storing signed value");
        }
    } else { // Load
        Word selected_halfword = memory.read_halfword_from_memory(aligned_address);
        Byte rotate_amount = (address & 1) * 8;
        selected_halfword = CpuALU().rotate_right(selected_halfword, rotate_amount);

        if (data_transfer.s == 0) { // Unsigned
            if (data_transfer.h == 0) { // Byte || Reserved for SWP

            } else { // Halfword || LDRH
                write_register(data_transfer.source_destination_register, selected_halfword);
            }
        } else { // Signed
            if (data_transfer.h == 0) { // Byte || LDRSB
                Byte selected_byte = memory.read_from_memory(address);
                write_register(data_transfer.source_destination_register, Utils::sign_extend(selected_byte, 8));
            } else { // Halfword || LDRSH
                write_register(data_transfer.source_destination_register, Utils::sign_extend(selected_halfword, 16 - rotate_amount));
            }
        }
    }
}

void ARM7TDMI::arm_opcode_block_data_transfer(Word opcode) {
    OpcodeBlockDataTransfer data_transfer = OpcodeBlockDataTransfer(opcode);
    Word base_address = read_register(data_transfer.base_register);

    if (is_priviledged() && data_transfer.s == 1) {
        warn("Block Data Transfer - not priviledged && s == 1");
    }
    if (data_transfer.base_register == REGISTER_PC) {
        warn("Block Data Transfer - base_register == PC");
    }

    Word offset = 0;
    Word write_back_address = base_address;
    bool pc_in_transfer_list = false;

    bool empty_register_list = data_transfer.register_list == 0b0;
    if (empty_register_list) {
        data_transfer.register_list = 0x8000;
        offset = 0x3C;
    }

    for (int i = 0; i < 16; i++) {
        bool transfer_register = Utils::read_bit(data_transfer.register_list, i);
        if (transfer_register) {
            offset += 4;
            if (i == 15) {
                pc_in_transfer_list = true;
            }
        }
    }
    write_back_address += data_transfer.u ? offset : -offset;

    Word current_address = base_address;
    if (data_transfer.u == 0) { // Down
        current_address = write_back_address;
    }

    bool pre_increment = data_transfer.u ? data_transfer.p : !data_transfer.p;

    RegisterSet * target_register_set = current_register_set();
    if ((data_transfer.s == 1) && (!pc_in_transfer_list) || (data_transfer.l == 0)) {
        target_register_set = &registers_user;
        if (data_transfer.w == 1) {
            warn("Block Data Transfer - writeback and using user register bank");
        }
    }

    if (data_transfer.w == 1) {
        write_register(data_transfer.base_register, write_back_address);
    }

    for (int i = 0; i < 16; i++) {
        bool transfer_register = Utils::read_bit(data_transfer.register_list, i);
        if (!transfer_register) {continue;}

        if (pre_increment == 1) {
            current_address += 4;
        }

        if (data_transfer.l == 0) { // Store | Register -> Memory
            bool stored_register_is_base_register = i == data_transfer.base_register;
            bool first_stored_register = Utils::read_bit_range(data_transfer.register_list, 0, i - 1) == 0;
            if (stored_register_is_base_register && first_stored_register) {
                write_word_to_memory(current_address, base_address);
            } else {
                Word register_value = *target_register_set->registers[i];
                if (i == REGISTER_PC) {
                    register_value += 12;
                }
                write_word_to_memory(current_address, register_value);
            }

        } else { // Load | Memory -> Register
            *target_register_set->registers[i] = read_word_from_memory(current_address);
            if (data_transfer.s == 1 && i == REGISTER_PC) {
                cpsr = current_register_set()->spsr;
            }
        }

        if (pre_increment == 0) {
            current_address += 4;
        }
    }
}

void ARM7TDMI::arm_opcode_swap(Word opcode) {
    OpcodeSwap swap = OpcodeSwap(opcode);
    if (swap.base_register == REGISTER_PC ||
        swap.source_register == REGISTER_PC ||
        swap.destination_register == REGISTER_PC
    ) {
        warn("Swap -- PC is a register");
    }

    Word address = read_register(swap.base_register);
    Word aligned_address = address & (~0b11);
    Word swap_address_value = read_word_from_memory(aligned_address);

    Word source_register_value = read_register(swap.source_register);
    if (swap.source_register == REGISTER_PC) {
        source_register_value += 12;
    }
    OpcodeSingleDataTransfer::store(this, address, source_register_value, swap.b); // Register -> Memory
    OpcodeSingleDataTransfer::load(this, address, swap_address_value, swap.destination_register, swap.b); // Memory -> Register
}