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



typedef struct OpcodeDataProcess {
     enum BitShiftType {
        LSL = 0,
        LSR = 1,
        ASR = 2,
        ROR = 3
    };

    enum InstructionType 
    {
        AND = 0x0,
        EOR = 0x1,
        SUB = 0x2,
        RSB = 0x3,
        ADD = 0x4,
        ADC = 0x5,
        SBC = 0x6,
        RSC = 0x7,
        TST = 0x8,
        TEQ = 0x9,
        CMP = 0xA,
        CMN = 0xB,
        ORR = 0xC,
        MOV = 0xD,
        BIC = 0xE,
        MVN = 0xF
    };

    enum OperationClass {
        ARITHMETIC,
        LOGICAL
    };

    unsigned int use_immediate_operand_2 : 1;
    unsigned int instruction_type : 4;
    unsigned int set_condition_codes : 1;
    unsigned int rn : 4;
    unsigned int rd : 4;

    // If (register_op_2)
    unsigned int shift_type : 2;
    unsigned int operand_2_register : 4;
    unsigned int shift_by_register : 1;

        // If (shift_by_register)
        unsigned int shift_register : 4;
        // Else (shift_by_immediate)
        unsigned int shift_immediate_amount : 5;

    // Else (immediate_op_2)
    unsigned int immediate_ror_shift : 4;
    unsigned int operand_2_immediate : 8;

    CpuALU alu;

    u_int64_t last_result;

    bool write_result;
    BitShiftType bit_shift_type;
    OperationClass operation_class;

    OpcodeDataProcess(Word opcode);

    Word calculate_immediate_op2( Byte immediate, unsigned int ror_shift);
    u_int64_t calculate_instruction(InstructionType instruction, Word rn, Word op2, bool c_flag);
    Word shift_op2(Word op2, Byte shift_amount, BitShiftType bit_shift_type, bool c_flag);
    void set_psr_flags(PSR * psr, u_int64_t result, OperationClass operation_class);

    Byte get_op_2_register_shift_amount(bool shift_by_register, Word shift_register_value);
    unsigned int calculate_pc_prefetch_offset();
} DataProcess;

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

Word OpcodeDataProcess::shift_op2(Word op2, Byte shift_amount, BitShiftType bit_shift_type, bool c_flag)
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
        default:
            printf("Instruction out of range (ALU)");
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
        op2 = opcode_class.calculate_immediate_op2(opcode_class.operand_2_immediate, opcode_class.immediate_ror_shift);
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

