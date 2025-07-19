#include <cstdio>
#include <limits.h>

#include "cpu.h"
#include "opcode_type.h"
#include "cpu_types.h"
#include "alu.h"

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

Word calculate_register_op2(ARM7TDMI * cpu, CpuALU * alu, Word opcode) {
    enum ShiftType {
            LSL = 0,
            LSR = 1,
            ASR = 2,
            ROR = 3
        };
        Byte shift_type = Utils::read_bit_range(opcode, 5, 6);
        Byte second_operand_register = Utils::read_bit_range(opcode, 0, 3);
        bool shift_by_register = Utils::read_bit(opcode, 4);

        Word second_operand_register_value = cpu->read_register(second_operand_register);
        Byte shift_amount;
        
        if (shift_by_register) {
            Byte shift_register = Utils::read_bit_range(opcode, 8, 11);
            if (shift_register == REGISTER_PC) printf("Invalid shift_register (ALU)");

            Word shift_register_value = cpu->read_register(shift_register);
            shift_amount = Utils::read_bit_range(shift_register_value, 0, 7);
            if (shift_amount == 0)
            {
                alu->carry_flag = cpu->cpsr.c;
                return second_operand_register_value;
            }
        } else { // Shift by immediate
            shift_amount = Utils::read_bit_range(opcode, 7, 11);
        }
        
        switch (shift_type)
        {
            case LSL:
                if (shift_amount == 0) {
                    alu->carry_flag = cpu->cpsr.c;
                    return second_operand_register_value;
                }
                return alu->logical_left_shift(second_operand_register_value, shift_amount);
            case LSR:
                if (shift_amount == 0)
                    shift_amount = 32;
                return alu->logical_right_shift(second_operand_register_value, shift_amount);
            case ASR:
                if (shift_amount == 0)
                    shift_amount = 32;
                return alu->arithmetic_right_shift(second_operand_register_value, shift_amount);
            case ROR:
                if (shift_amount == 0) 
                    return alu->rotate_right_extended(second_operand_register_value, cpu->cpsr.c);
                return alu->rotate_right(second_operand_register_value, shift_amount);
            default:
                printf("Invalid shift type (ALU)");
                return 0;
        }
}

u_int64_t data_processing_instruction(ARM7TDMI * cpu, CpuALU * alu, Byte instruction, Word first_register_operand, Word second_operand)
{
    u_int64_t result;
    bool set_destination = true;

    switch (instruction)
    {
        case 0x0: // AND
            result = first_register_operand & second_operand;
            break;
        case 0x1: // EOR
            result = first_register_operand ^ second_operand;
            break;
        case 0x2: // SUB 
            result = alu->subtract(1, first_register_operand, second_operand);
            break;
        case 0x3: // RSB
            result = alu->subtract(1, second_operand, first_register_operand);
            break;
        case 0x4: // ADD
            result = alu->add(2, first_register_operand, second_operand);
            break;
        case 0x5: // ADC
            result = alu->add(3, first_register_operand, second_operand, cpu->cpsr.c);
            break;
        case 0x6: // SBC
            result = alu->subtract(2, first_register_operand, second_operand, -(cpu->cpsr.c - 1));
            break;
        case 0x7: // RSC
            result = alu->subtract(2, second_operand, first_register_operand, -(cpu->cpsr.c - 1));
            break;
        case 0x8: // TST
            result = first_register_operand & second_operand;
            set_destination = false;
            break;
        case 0x9: // TEQ
            result = first_register_operand ^ second_operand;
            set_destination = false;
            break;
        case 0xA: // CMP
            result = alu->subtract(1, first_register_operand, second_operand);
            set_destination = false;
            break;
        case 0xB: // CMN
            result = alu->add(2, first_register_operand, second_operand);
            set_destination = false;
            break;
        case 0xC: // ORR
            result = first_register_operand | second_operand;
            break;
        case 0xD: // MOV
            result = second_operand;
            break;
        case 0xE: // BIC
            result = first_register_operand & (~second_operand);
            break;
        case 0xF: // MVN
            result = ~second_operand;
            break;
        default:
            printf("Instruction out of range (ALU)");
    }

    return result;
}

bool is_logical(DataProcessingInstructionType instruction_type) {
    return (
        instruction_type == DataProcessingInstructionType::AND ||
        instruction_type == DataProcessingInstructionType::EOR ||
        instruction_type == DataProcessingInstructionType::TST ||
        instruction_type == DataProcessingInstructionType::TEQ ||
        instruction_type == DataProcessingInstructionType::ORR ||
        instruction_type == DataProcessingInstructionType::MOV ||
        instruction_type == DataProcessingInstructionType::BIC ||
        instruction_type == DataProcessingInstructionType::MVN
    );
}

bool does_instruction_set_destination(
    DataProcessingInstructionType instruction_type
) {
    return !(
        instruction_type == DataProcessingInstructionType::TST ||
        instruction_type == DataProcessingInstructionType::TEQ ||
        instruction_type == DataProcessingInstructionType::CMP ||
        instruction_type == DataProcessingInstructionType::CMN
    );
}

void ARM7TDMI::opcode_data_processing(Word opcode) 
{   
    bool immediate_operand_2 = Utils::read_bit(opcode, 25);
    Byte instruction = Utils::read_bit_range(opcode, 21, 24);
    bool set_condition_codes = Utils::read_bit(opcode, 20);

    Byte first_operand_register = Utils::read_bit_range(opcode, 16, 19);
    Byte destination_register = Utils::read_bit_range(opcode, 12, 15);

    Word first_register_value = read_register(first_operand_register);
    Word second_operand;

    CpuALU alu;

    if (!immediate_operand_2) // Register OP2
    {
        second_operand = calculate_register_op2(this, &alu, opcode);
    } 
    else 
    {
        Byte immediate_shift_amount = Utils::read_bit_range(opcode, 8, 11) * 2;
        Byte second_operand_immediate = Utils::read_bit_range(opcode, 0, 7);
        Word zero_extended_immediate = second_operand_immediate & 0xFF;

        Word rotated_immediate = alu.rotate_right(zero_extended_immediate, immediate_shift_amount);
        second_operand = rotated_immediate;
    }

    u_int64_t result = data_processing_instruction(this, &alu, instruction, first_register_value, second_operand);;
    int64_t result_signed = result;
    Word result_word = result & UINT32_MAX;

    DataProcessingInstructionType instruction_type = (DataProcessingInstructionType)instruction;
    
    bool set_destination = does_instruction_set_destination(instruction_type);

    if (set_destination)    
    {
        write_register(destination_register, result_word);
    }
    
    if (set_condition_codes)
    {
        cpsr.c = alu.carry_flag;
        cpsr.z = result == 0;

        bool is_logical_operation = is_logical(instruction_type);

        cpsr.n = Utils::read_bit(result_word, 31);
        if (!is_logical_operation)
        {
            if (result_signed > INT_MAX || result_signed < INT_MIN)
                cpsr.v = 1;
            else
                cpsr.v = 0;
        }
    }
}

