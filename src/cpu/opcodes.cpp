#include "cpu.h"
#include "opcode_type.h"
#include "cpu_types.h"

#include <cstdio>

void ARM7TDMI::opcode_branch(Word opcode)
{
    Word pc_with_prefetch_offset = read_register(REGISTER_PC) + 8;
    int32_t offset = Utils::read_bit_range(&opcode, 0, 23);
    bool link_bit = Utils::read_bit(&opcode, 24);

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
    Word register_number = Utils::read_bit_range(&opcode, 0, 3);
    Word register_value = read_register(register_number);
    bool bit_one = Utils::read_bit(&register_number, 0);

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
    bool immediate_operand_2 = Utils::read_bit(&opcode, 25);
    Byte intruction = Utils::read_bit_range(&opcode, 21, 24);
    bool set_condition_codes = Utils::read_bit(&opcode, 20);

    Byte first_operand_register = Utils::read_bit_range(&opcode, 16, 19);
    Byte destination_register = Utils::read_bit_range(&opcode, 12, 15);
    Word second_operand;

    
    if (!immediate_operand_2) // Register OP2
    {
        enum ShiftType {
            LSL = 0,
            LSR = 1,
            ASR = 2,
            ROR = 3
        };
        Byte shift_type = Utils::read_bit_range(&opcode, 5, 6);
        Byte second_operand_register = Utils::read_bit_range(&opcode, 0, 3);

        bool shift_by_register = Utils::read_bit(&opcode, 4);
        Byte shift_amount;

        if (shift_by_register) {
            Byte shift_register = Utils::read_bit_range(&opcode, 8, 11);
            if (shift_register == REGISTER_PC) printf("Invalid shift_register (ALU)");

            Word shift_register_value = read_register(shift_register);
            shift_amount = Utils::read_bit_range(&shift_register_value, 0, 7);
        } else {
            shift_amount = Utils::read_bit_range(&opcode, 7, 11);
        }
        
        Word second_operand_register_value = read_register(second_operand_register);

        switch (shift_type)
        {
            case LSL:
                second_operand = second_operand_register_value << shift_amount;
                break;
            case LSR:
                second_operand = Utils::logical_right_shift(second_operand_register_value, shift_amount, 32);
                break;
            case ASR:
                second_operand = Utils::arithmetic_right_shift(second_operand_register_value, shift_amount);
                break;
            case ROR:
                second_operand = Utils::rotate_right(second_operand_register_value, shift_amount, 32);
                break;
            default:
                printf("Invalid shift type (ALU)");
                break;
        }
    } 
    else 
    {
        Byte immediate_shift_amount = Utils::read_bit_range(&opcode, 8, 11) * 2;
        Byte second_operand_immediate = Utils::read_bit_range(&opcode, 0, 7);
        Word zero_extended_immediate = second_operand_immediate & 0xFF;

        Word rotated_immediate = Utils::rotate_right(zero_extended_immediate, immediate_shift_amount, 32);
        second_operand = rotated_immediate;
    }


    Byte first_register_value = read_register(first_operand_register);
    Word result;

    bool set_destination = true;
    switch (intruction)
    {
        case 0x0: // AND
            result = first_register_value & second_operand;
            break;
        case 0x1: // EOR
            result = first_register_value ^ second_operand;
            break;
        case 0x2: // SUB
            result = first_register_value - second_operand;
            break;
        case 0x3: // RSB
            result = second_operand - first_register_value;
            break;
        case 0x4: // ADD
            result = first_register_value + second_operand;
            break;
        case 0x5: // ADC
            result = first_register_value + second_operand + cpsr.c;
            break;
        case 0x6: // SBC
            result = first_register_value - second_operand + (cpsr.c - 1);
            break;
        case 0x7: // RSC
            result = second_operand - first_register_value + (cpsr.c - 1);
            break;
        case 0x8: // TST
            result = first_operand_register & second_operand;
            set_destination = false;
            break;
        case 0x9: // TEQ
            result = first_operand_register ^ second_operand;
            set_destination = false;
            break;
        case 0xA: // CMP
            result = first_operand_register - second_operand;
            set_destination = false;
            break;
        case 0xB: // CMN
            result = first_operand_register + second_operand;
            set_destination = false;
            break;
        case 0xC: // ORR
            result = first_register_value | second_operand;
            break;
        case 0xD: // MOV
            result = second_operand;
            break;
        case 0xE: // BIC
            result = first_register_value & (~second_operand);
            break;
        case 0xF: // MVN
            result = ~second_operand;
            break;
        default:
            printf("Instruction out of range (ALU)");
    }

    if (set_destination) 
    {
        write_register(destination_register, result);
    }
    
}

