#include "data_processing.h"
#include "src/utils.h"

#include <limits.h>

OpcodeDataProcess::OpcodeDataProcess(Word opcode) 
{
    use_immediate_operand_2 = Utils::read_bit(opcode, 25);
    instruction_type = Utils::read_bit_range(opcode, 21, 24);
    set_condition_codes = Utils::read_bit(opcode, 20);
    rn = Utils::read_bit_range(opcode, 16, 19);
    rd = Utils::read_bit_range(opcode, 12, 15);

    shift_register = Utils::read_bit_range(opcode, 8, 11);
    shift_immediate_amount = Utils::read_bit_range(opcode, 7, 11);
    
    shift_type = Utils::read_bit_range(opcode, 5, 6);
    shift_by_register == Utils::read_bit(opcode, 4);
    operand_2_register = Utils::read_bit_range(opcode, 0, 3);

    immediate_ror_shift = Utils::read_bit_range(opcode, 8, 11);
    operand_2_immediate = Utils::read_bit_range(opcode, 0, 7);

    bit_shift_type = (BitShiftType)shift_type;

    if (
        instruction_type == TST ||
        instruction_type == TEQ ||
        instruction_type == CMP ||
        instruction_type == CMN
    )   {
        write_result = false;
    } else {
        write_result = true;
    }

    if (
        instruction_type == AND ||
        instruction_type == EOR ||
        instruction_type == TST ||
        instruction_type == TEQ ||
        instruction_type == ORR ||
        instruction_type == MOV ||
        instruction_type == BIC ||
        instruction_type == MVN
    ) {
        operation_class = LOGICAL;
    } else {
        operation_class = ARITHMETIC;
    }
}

Word OpcodeDataProcess::calculate_immediate_op2( Byte immediate, unsigned int ror_shift)
{
        Word zero_extended_immediate = immediate & 0xFF;
        Word rotated_immediate = alu.rotate_right(zero_extended_immediate, ror_shift);

        return rotated_immediate;
}

Word OpcodeDataProcess::shift_op2(CpuALU alu, Word op2, Byte shift_amount, BitShiftType bit_shift_type, bool c_flag)
{
    switch (bit_shift_type)
    {
        case LSL:
            if (shift_amount == 0) {
                alu.carry_flag = c_flag;
                return op2;
            }
            return alu.logical_left_shift(op2, shift_amount);
        case LSR:
            if (shift_amount == 0)
                shift_amount = 32;
            return alu.logical_right_shift(op2, shift_amount);
        case ASR:
            if (shift_amount == 0)
                shift_amount = 32;
            return alu.arithmetic_right_shift(op2, shift_amount);
        case ROR:
            if (shift_amount == 0) 
                return alu.rotate_right_extended(op2, c_flag);
            return alu.rotate_right(op2, shift_amount);
    }
}

u_int64_t OpcodeDataProcess::calculate_instruction(InstructionType instruction, Word rn, Word op2, bool c_flag)
{
    u_int64_t result;

    switch (instruction)
    {
        case 0x0: // AND
            result = rn & op2;
            break;
        case 0x1: // EOR
            result = rn ^ op2;
            break;
        case 0x2: // SUB 
            result = alu.subtract(1, rn, op2);
            break;
        case 0x3: // RSB
            result = alu.subtract(1, op2, rn);
            break;
        case 0x4: // ADD
            result = alu.add(2, rn, op2);
            break;
        case 0x5: // ADC
            result = alu.add(3, rn, op2, c_flag);
            break;
        case 0x6: // SBC
            result = alu.subtract(2, rn, op2, -(c_flag - 1));
            break;
        case 0x7: // RSC
            result = alu.subtract(2, op2, rn, -(c_flag - 1));
            break;
        case 0x8: // TST
            result = rn & op2;
            break;
        case 0x9: // TEQ
            result = rn ^ op2;
            break;
        case 0xA: // CMP
            result = alu.subtract(1, rn, op2);
            break;
        case 0xB: // CMN
            result = alu.add(2, rn, op2);
            break;
        case 0xC: // ORR
            result = rn | op2;
            break;
        case 0xD: // MOV
            result = op2;
            break;
        case 0xE: // BIC
            result = rn & (~op2);
            break;
        case 0xF: // MVN
            result = ~op2;
            break;
    }

    last_result = result;
    
    return result;
}

Byte OpcodeDataProcess::get_op_2_register_shift_amount(bool shift_by_register, Word shift_register_value)
{
    if (shift_by_register) {
        return Utils::read_bit_range(shift_register_value, 0, 7);
    } else {
        return shift_immediate_amount;
    }
}

unsigned int OpcodeDataProcess::calculate_pc_prefetch_offset() 
{
    if (!use_immediate_operand_2 && shift_by_register) {
        return 12;
    } else {
        return 8;
    }
}

void OpcodeDataProcess::set_psr_flags(PSR * psr, u_int64_t result, OperationClass operation_class) 
{
    psr->c = alu.carry_flag;
    psr->z = result == 0;
    psr->n = Utils::read_bit(result, 31);

    if (operation_class == ARITHMETIC)
    {
        int64_t result_signed = result;
        if (result_signed > INT_MAX || result_signed < INT_MIN)
            psr->v = 1;
        else
            psr->v = 0;
    }
}