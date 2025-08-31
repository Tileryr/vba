#include "data_processing.h"
#include "src/utils.h"

#include <limits.h>
OpcodeDataProcess::OpcodeDataProcess() {}
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
    shift_by_register = Utils::read_bit(opcode, 4);
    operand_2_register = Utils::read_bit_range(opcode, 0, 3);

    immediate_ror_shift = Utils::read_bit_range(opcode, 8, 11);
    operand_2_immediate = Utils::read_bit_range(opcode, 0, 7);
}

Word OpcodeDataProcess::calculate_immediate_op2( Byte immediate, unsigned int ror_shift)
{
        Word zero_extended_immediate = immediate & 0xFF;
        Word rotated_immediate = alu.rotate_right(zero_extended_immediate, ror_shift);

        return rotated_immediate;
}

Word OpcodeDataProcess::shift_op2(CpuALU * alu, Word op2, Byte shift_amount, BitShiftType bit_shift_type, bool c_flag)
{
    switch (bit_shift_type)
    {
        case LSL:
            if (shift_amount == 0) {
                alu->carry_flag = c_flag;
                return op2;
            }
            return alu->logical_left_shift(op2, shift_amount);
        case LSR:
            if (shift_amount == 0)
                shift_amount = 32;
            return alu->logical_right_shift(op2, shift_amount);
        case ASR:
            if (shift_amount == 0)
                shift_amount = 32;
            return alu->arithmetic_right_shift(op2, shift_amount);
        case ROR:
            if (shift_amount == 0) 
                return alu->rotate_right_extended(op2, c_flag);
            return alu->rotate_right(op2, shift_amount);
    }
}

u_int64_t OpcodeDataProcess::calculate_instruction(CpuALU * alu, InstructionType instruction, Word rn, Word op2, bool c_flag)
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
            result = alu->subtract(1, rn, op2);
            break;
        case 0x3: // RSB
            result = alu->subtract(1, op2, rn);
            break;
        case 0x4: // ADD
            result = alu->add(2, rn, op2);
            break;
        case 0x5: // ADC
            result = alu->add(3, rn, op2, c_flag);
            break;
        case 0x6: // SBC
            result = alu->subtract(2, rn, op2, -(c_flag - 1));
            break;
        case 0x7: // RSC
            result = alu->subtract(2, op2, rn, -(c_flag - 1));
            break;
        case 0x8: // TST
            result = rn & op2;
            break;
        case 0x9: // TEQ
            result = rn ^ op2;
            break;
        case 0xA: // CMP
            result = alu->subtract(1, rn, op2);
            break;
        case 0xB: // CMN
            result = alu->add(2, rn, op2);
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

bool OpcodeDataProcess::do_write_result(InstructionType instruction) {
    return !(
        instruction == TST ||
        instruction == TEQ ||
        instruction == CMP ||
        instruction == CMN
    );
}

OpcodeDataProcess::OperationClass OpcodeDataProcess::operation_class(InstructionType instruction) {
    if (
        instruction == AND ||
        instruction == EOR ||
        instruction == TST ||
        instruction == TEQ ||
        instruction == ORR ||
        instruction == MOV ||
        instruction == BIC ||
        instruction == MVN
    ) {
        return LOGICAL;
    } else {
        return ARITHMETIC;
    }
}

void OpcodeDataProcess::set_psr_flags(CpuALU * alu, PSR * psr, u_int64_t result) 
{
    psr->c = alu->carry_flag;
    psr->z = result == 0;
    psr->n = Utils::read_bit(result, 31);
}

bool OpcodeDataProcess::get_overflow_flag(Word op1, Word op2, u_int64_t result, bool subtraction) {
    if (subtraction) {op2 = (UINT32_MAX - op2);}

    return ((op1^result)&(op2^result)&(1 << 31)) != 0;
}

void OpcodeDataProcess::run(ARM7TDMI * cpu) {
    Word rn_value = cpu->read_register(this->rn);
    Word op2;

    if (!use_immediate_operand_2) {
        op2 = cpu->read_register(operand_2_register);
        if (operand_2_register == REGISTER_PC)
            op2 += calculate_pc_prefetch_offset();

        Word shift_register_value = cpu->read_register(shift_register);
        Byte shift_amount = get_op_2_register_shift_amount(shift_by_register, shift_register_value);

        if (shift_by_register && shift_amount == 0) {
            alu.carry_flag = cpu->cpsr.c;
        } else {
            op2 = shift_op2(&alu, op2, shift_amount, (BitShiftType)shift_type, cpu->cpsr.c);
        }
        
    } else {
        op2 = calculate_immediate_op2(operand_2_immediate, immediate_ror_shift * 2);
    } 

    if (this->rn == REGISTER_PC) {
        rn_value += calculate_pc_prefetch_offset();
    }

    InstructionType instruction_type = (InstructionType)this->instruction_type;
    u_int64_t result = calculate_instruction(&alu, instruction_type, rn_value, op2, cpu->cpsr.c);

    if (set_condition_codes) {
        if (rd == REGISTER_PC) {
            cpu->cpsr = cpu->current_register_set()->spsr;
            return;
        }

        set_psr_flags(&alu, &cpu->cpsr, result);
        if (operation_class(instruction_type) == OpcodeDataProcess::ARITHMETIC) {
            bool sub = (
                instruction_type == OpcodeDataProcess::SUB ||
                instruction_type == OpcodeDataProcess::SBC ||
                instruction_type == OpcodeDataProcess::RSB ||
                instruction_type == OpcodeDataProcess::RSC ||
                instruction_type == OpcodeDataProcess::CMP 
            );

            cpu->cpsr.v = get_overflow_flag(rn_value, op2, result, sub);
        }
    }

    if (do_write_result(instruction_type)) {
        cpu->write_register(rd, result & UINT32_MAX);
    }
}

OpcodeDataProcessingBuilder::OpcodeDataProcessingBuilder(Word instruction_type, bool set_condition_codes) {
    product = OpcodeDataProcess();
    product.instruction_type = instruction_type;
    product.set_condition_codes = set_condition_codes;
}

OpcodeDataProcessingBuilder& OpcodeDataProcessingBuilder::set_destination_register(Word destination_register) {
    product.rd = destination_register;
    return *this;
}
OpcodeDataProcessingBuilder& OpcodeDataProcessingBuilder::set_source_register(Word source_register) {
    product.rn = source_register;
    return *this;
}

OpcodeDataProcessingBuilder& OpcodeDataProcessingBuilder::set_immediate_op2(Byte immediate, Word ror_shift) {
    product.use_immediate_operand_2 = true;
    product.operand_2_immediate = immediate;
    product.immediate_ror_shift = ror_shift;
    return *this;
}
OpcodeDataProcessingBuilder& OpcodeDataProcessingBuilder::set_register_op2(Byte op2_register, Byte shift_type) {
    product.use_immediate_operand_2 = false;
    product.operand_2_register = op2_register;
    product.shift_type = shift_type;
    return *this;
}

OpcodeDataProcessingBuilder& OpcodeDataProcessingBuilder::set_register_op2_shift_register(Byte shift_register) {
    product.shift_by_register = true;
    product.shift_register = shift_register;
    return *this;
}
OpcodeDataProcessingBuilder& OpcodeDataProcessingBuilder::set_register_op2_shift_immediate(Byte shift_immediate_amount) {
    product.shift_by_register = false;
    product.shift_immediate_amount = shift_immediate_amount;
    return *this;
}

OpcodeDataProcess OpcodeDataProcessingBuilder::get_product() {
    return product;
}
