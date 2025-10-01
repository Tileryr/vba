#include <SDL3/SDL.h>
#include <cstdio>
#include <limits.h>
#include <stdlib.h>

#include "../cpu_types.h"
#include "../alu.h"
#include "../cpu.h"

#include "opcode_types.h"

#include "src/cpu/opcodes/arm/branch.h"
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
    OpcodeBranch branch = OpcodeBranch(opcode);
    Word pc_with_prefetch_offset = read_register(REGISTER_PC) + 8;

    Word offset = branch.offset;
    offset = offset << 2;

    if (branch.link_bit)
    {
        Word return_address = pc_with_prefetch_offset - 4;
        write_register(REGISTER_LR, return_address);
    }

    OpcodeBranch::branch(this, pc_with_prefetch_offset, offset, 26);
}

void ARM7TDMI::arm_opcode_branch_exchange(Word opcode)
{
    OpcodeBranchExchange(opcode).run(this);
}

void ARM7TDMI::arm_opcode_software_interrupt(Word opcode)
{
    Word comment_field = Utils::read_bit_range(opcode, 0, 23);
    emulate_software_interrupt(comment_field>>16);
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
        result_signed = (int64_t)rm_value_u * (int64_t)rs_value_u;

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
    data_transfer.run(this);
}

void ARM7TDMI::arm_opcode_half_word_signed_data_transfer(Word opcode) {
    OpcodeHalfWordSignedDataTransfer data_transfer = OpcodeHalfWordSignedDataTransfer(opcode);
    data_transfer.run(this);
}

void ARM7TDMI::arm_opcode_block_data_transfer(Word opcode) {
    OpcodeBlockDataTransfer data_transfer = OpcodeBlockDataTransfer(opcode);
    data_transfer.run(this);
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