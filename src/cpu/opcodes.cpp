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
        op2 = opcode_class.shift_op2(op2, shift_amount, opcode_class.bit_shift_type, cpsr.c);
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