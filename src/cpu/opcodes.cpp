#include <cstdio>
#include <limits.h>
#include <stdlib.h>

#include "cpu.h"
#include "opcode_type.h"
#include "cpu_types.h"
#include "alu.h"

#include "./opcodes/data_processing.h"
#include "./opcodes/multiply.h"
#include "./opcodes/multiply_long.h"
#include "./opcodes/psr_transfer.h"
#include "./opcodes/single_data_transfer.h"

void ARM7TDMI::opcode_branch(Word opcode)
{
    Word pc_with_prefetch_offset = read_register(REGISTER_PC) + 8;
    int32_t offset = Utils::read_bit_range(opcode, 0, 23);
    bool link_bit = Utils::read_bit(opcode, 24);

    offset = offset << 2;
    offset = Utils::sign_extend(offset, 26);

    if (link_bit)
    {
        Word return_address = pc_with_prefetch_offset - 4;
        write_register(REGISTER_LS, return_address);
    }

    Word new_address = pc_with_prefetch_offset + offset;
    write_register(REGISTER_PC, new_address);
}

void ARM7TDMI::opcode_branch_exchange(Word opcode)
{
    Word register_number = Utils::read_bit_range(opcode, 0, 3);
    Word register_value = read_register(register_number);
    bool bit_one = Utils::read_bit(register_number, 0);

    if (bit_one)
    {
        cpsr.t = STATE_THUMB;
        write_register(REGISTER_PC, register_value - 1);
    } else 
    {
        cpsr.t = STATE_ARM;
        write_register(REGISTER_PC, register_value);
    }
}

void ARM7TDMI::opcode_software_interrupt(Word opcode)
{
    run_exception(EXCEPTION_SOFTWARE_INTERRUPT);
}

void ARM7TDMI::opcode_undefined_intruction(Word opcode)
{
    run_exception(EXCEPTION_UNDEFINED);
}

void ARM7TDMI::opcode_data_processing(Word opcode) 
{   
    DataProcess opcode_class = DataProcess(opcode);
    Word rn = read_register(opcode_class.rn);
    Word op2;

    if (!opcode_class.use_immediate_operand_2) {
        op2 = read_register(opcode_class.operand_2_register);
        if (op2 == REGISTER_PC)
            op2 += opcode_class.calculate_pc_prefetch_offset();

        Word shift_register_value = read_register(opcode_class.shift_register);
        Byte shift_amount = opcode_class.get_op_2_register_shift_amount(opcode_class.shift_by_register, shift_register_value);
        op2 = opcode_class.shift_op2(opcode_class.alu, op2, shift_amount, opcode_class.bit_shift_type, cpsr.c);
    } else {
        op2 = opcode_class.calculate_immediate_op2(opcode_class.operand_2_immediate, opcode_class.immediate_ror_shift * 2);
    } 

    if (rn == REGISTER_PC) {
        rn += opcode_class.calculate_pc_prefetch_offset();
    }

    opcode_class.calculate_instruction((OpcodeDataProcess::InstructionType)opcode_class.instruction_type, rn, op2, cpsr.c);

    if (opcode_class.set_condition_codes) {
        if (opcode_class.rd == REGISTER_PC) {
            cpsr = current_register_set()->spsr;
            return;
        }
         opcode_class.set_psr_flags(&cpsr, opcode_class.last_result, opcode_class.operation_class);
    }
   
    if (opcode_class.write_result) {
        write_register(opcode_class.rd, opcode_class.last_result & UINT32_MAX);
    }
}

void ARM7TDMI::opcode_multiply(Word opcode)
{
    Multiply multiply = Multiply(opcode);
    Word destination_value;

    Word rn_value = read_register(multiply.rn);
    Word rs_value = read_register(multiply.rs);
    Word rm_value = read_register(multiply.rm);

    if (
        multiply.register_destination == REGISTER_PC ||
        multiply.rn == REGISTER_PC ||
        multiply.rs == REGISTER_PC ||
        multiply.rm == REGISTER_PC 
    ) {
        warn("Multiply - Invalid register value (PC)");
    }

    if ( multiply.register_destination == multiply.rm ) {
        warn("Multiply - Invalid register value (rd == rm)");
    }

    if (multiply.accumulate) {
        destination_value = (rm_value * rs_value) + rn_value;
    } else { // Multiply Only
        destination_value = rm_value * rs_value;
    }

    if (multiply.set_condition_codes) {
        cpsr.n = Utils::read_bit(31, destination_value);
        cpsr.z = destination_value == 0;
        cpsr.c = rand() & 1;
        cpsr.v; // Unaffected
    }

    write_register(multiply.register_destination, destination_value);
};

void ARM7TDMI::opcode_multiply_long(Word opcode)
{
    MultiplyLong multiply_long = MultiplyLong(opcode);
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
        int64_t rs_value_u = read_register(multiply_long.rs);
        int64_t rm_value_u = read_register(multiply_long.rm);
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

    if (multiply_long.set_condition_codes)
    {
        cpsr.n = Utils::read_bit(destination_high_value, 31);
        cpsr.z = result == 0;
        cpsr.c = rand() & 1;
        cpsr.v = rand() & 1;
    }
}
 
void ARM7TDMI::opcode_psr_transfer(Word opcode)
{
    OpcodePsrTransfer psr_transfer = OpcodePsrTransfer(opcode);
    PSR * target_psr = psr_transfer.psr == 0 
        ? &cpsr 
        : &current_register_set()->spsr;

    if (psr_transfer.is_msr_instruction) {
        Word source_register_value = read_register(psr_transfer.register_source);
        if (psr_transfer.only_write_flag) {
            Word write_value;
            if (psr_transfer.immediate_operand) {
                CpuALU alu;
                Byte immediate = psr_transfer.immediate_value;
                Byte immediate_rotate = psr_transfer.immediate_rotate * 2;
                write_value = alu.rotate_right(immediate, immediate_rotate);
            } else {
                write_value = source_register_value;
            }
            Word new_psr_value = target_psr->value();
            Word new_flag_bits = Utils::read_bit_range(write_value, 28, 31);
            Utils::write_bit_range(&new_psr_value, 28, 31, new_flag_bits);
            target_psr->write_value(new_psr_value);
        } else { // Write everything
            target_psr->write_value(source_register_value);
        }
    } else { // MRS
        Byte destination_register = psr_transfer.register_destination;
        write_register(destination_register, target_psr->value());  
    }
};

void ARM7TDMI::opcode_single_data_transfer(Word opcode)
{
    OpcodeSingleDataTransfer data_transfer = OpcodeSingleDataTransfer(opcode);
    Word address;
    Word offset;

    if (data_transfer.i == 1) { // Offset = Shifted Register
        Word offset_register_value = read_register(data_transfer.offset_register);
        OpcodeDataProcess::BitShiftType shift_type = static_cast<OpcodeDataProcess::BitShiftType>(data_transfer.register_shift_type);
        offset = OpcodeDataProcess::shift_op2(CpuALU(), offset_register_value, data_transfer.register_shift_amount, shift_type, cpsr.c);
    } else { // Offset = immediate value
        offset = data_transfer.offset_immediate;
    }

    Word offseted_base_register;
    Word base_register_value = read_register(data_transfer.base_register);

    if (data_transfer.base_register == REGISTER_PC) {
        if (data_transfer.w) {
            warn("Single Data Transfer - Base register == PC && writeback");
        }
        base_register_value += 8;
    }
    if (data_transfer.offset_register == REGISTER_PC) {
        warn("Single Data Transfer - Offset register == PC");
    }

    if (data_transfer.u == 0) { // Down - Subtract
        offseted_base_register = base_register_value - offset;
    } else { // Up - Add
        offseted_base_register = base_register_value + offset;
    }

    if (data_transfer.p == 1) { // Pre-Indexed
        address = offseted_base_register;
    } else {
        address = base_register_value;
    }

    if (data_transfer.w == 1) { // Write-back
        write_register(
            data_transfer.base_register, 
            address
        );
    }

    Word word_at_address;

    // For little endian only
    if (data_transfer.l == 0) { // Store
        Word stored_register = read_register(data_transfer.source_destination_register);
        if (data_transfer.source_destination_register == REGISTER_PC) {
            stored_register += 12;
        }
        
        if (data_transfer.b == 1) { // Byte
            Byte stored_byte = stored_register & 0xFF;
            memory[address] = stored_byte;
        } else { // Word
            Word aligned_address = address & (~0b11);
            memory[address + 0] = (stored_register >> 0) & 0xFF;
            memory[address + 1] = (stored_register >> 8) & 0xFF;
            memory[address + 2] = (stored_register >> 16) & 0xFF;
            memory[address + 3] = (stored_register >> 24) & 0xFF;
            Utils::current_word_at_memory(&memory[aligned_address], endian_type);
        }
    } else { // Load
        if (data_transfer.b == 1) { // Byte
            write_register(
            data_transfer.source_destination_register,
            memory[address]
            );
        } else { // Word
            u_int16_t aligned_offset = address % 4;
            Word aligned_address = address & (~0b11);
            Word word_at_address = Utils::current_word_at_memory(&memory[aligned_address], endian_type);
            Word rotated_word_at_address = CpuALU().rotate_right(word_at_address, (4 - aligned_offset) * 8); // Rotate it so that the first byte is the target byte in the address. 
            write_register(
                data_transfer.source_destination_register,
                rotated_word_at_address
            );
        }
    }

    if (data_transfer.p == 0) { // Post-indexed
        write_register(
            data_transfer.base_register, 
            offseted_base_register
        );
    }
}
