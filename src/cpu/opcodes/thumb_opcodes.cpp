#include "src/cpu/cpu.h"
#include "src/cpu/opcodes/arm/data_processing.h"

#include "src/cpu/opcodes/arm/branch_exchange.h"
#include "opcode_types.h"

void ARM7TDMI::thumb_opcode_move_shifted_register(HalfWord opcode) {
    Byte sub_opcode = Utils::read_bit_range(opcode, 11, 12);
    Byte offset = Utils::read_bit_range(opcode, 6, 10);
    Byte source_register = Utils::read_bit_range(opcode, 3, 5);
    Byte destination_register = Utils::read_bit_range(opcode, 0, 2);

    OpcodeDataProcess::BitShiftType shift_type;

    switch (sub_opcode)
    {
        case 0:
            shift_type = OpcodeDataProcess::LSL;
            break;
        case 1:
            shift_type = OpcodeDataProcess::LSR;
            break;
        case 2:
            shift_type = OpcodeDataProcess::ASR;
            break;
    }
}

void ARM7TDMI::thumb_opcode_move_compare_add_subtract(HalfWord opcode) {
    CpuALU alu;

    Byte opcode_instruction_type = Utils::read_bit_range(opcode, 11, 12);
    Byte source_destination_register = Utils::read_bit_range(opcode, 8, 10);
    Byte immediate = Utils::read_bit_range(opcode, 0, 7);

    OpcodeDataProcess::InstructionType instruction_type;
    OpcodeDataProcess::OperationClass operation_class = OpcodeDataProcess::ARITHMETIC;

    switch (opcode_instruction_type)
    {
        case 0: 
            instruction_type = OpcodeDataProcess::MOV; 
            operation_class = OpcodeDataProcess::LOGICAL; 
            break;
        case 1: instruction_type = OpcodeDataProcess::CMP; break;
        case 2: instruction_type = OpcodeDataProcess::ADD; break;
        case 3: instruction_type = OpcodeDataProcess::SUB; break;
    }

    u_int64_t result = OpcodeDataProcess::calculate_instruction(
        &alu, 
        instruction_type, 
        read_register(source_destination_register), 
        immediate, 
        cpsr.c
    );

    OpcodeDataProcess::set_psr_flags(&alu, &cpsr, result);

    if (OpcodeDataProcess::do_write_result(instruction_type)) {
        write_register(source_destination_register, result & UINT32_MAX);
    }
}

void ARM7TDMI::thumb_opcode_hi_register_operations_branch_exchange(HalfWord opcode, bool * increment_pc) {
    Byte opcode_instruction_type = Utils::read_bit_range(opcode, 8, 9);
    bool hi_operand_flag_1 = Utils::read_bit(opcode, 7);
    bool hi_operand_flag_2 = Utils::read_bit(opcode, 6);

    Byte source_register = Utils::read_bit_range(opcode, 3, 5);
    Byte destination_register = Utils::read_bit_range(opcode, 0, 2);
    Byte source_hi_register = source_register + 8;
    Byte destination_hi_register = destination_register + 8;
    
    Byte target_destination_register = hi_operand_flag_1 ? destination_hi_register : destination_register;
    Byte target_source_register = hi_operand_flag_2 ? source_hi_register : source_register;

    if (opcode_instruction_type == 3) { // BX
        OpcodeBranchExchange branch_exchange;
        OpcodeBranchExchange::build(&branch_exchange, target_source_register);
        *increment_pc = false;
        branch_exchange.run(this);
        return;
    }

    CpuALU alu;
    OpcodeDataProcess::InstructionType instruction_type;

    switch (opcode_instruction_type)
    {
        case 0: // ADD
            instruction_type = OpcodeDataProcess::ADD;
            break;
        case 1: // CMP
            instruction_type = OpcodeDataProcess::CMP;
            break;
        case 2: // MOV
            instruction_type = OpcodeDataProcess::MOV;
            break;
    }

    u_int64_t result = OpcodeDataProcess::calculate_instruction(
        &alu, 
        instruction_type, 
        read_register(target_destination_register), 
        read_register(target_source_register), 
        cpsr.c
    );

    if (instruction_type == OpcodeDataProcess::CMP) {
        OpcodeDataProcess::set_psr_flags(&alu, &cpsr, result);
        return;
    }

    write_register(target_destination_register, result & UINT32_MAX);
}

void ARM7TDMI::thumb_opcode_load_address(HalfWord opcode) {
    bool sp = Utils::read_bit(opcode, 11);
    Byte destination_register = Utils::read_bit_range(opcode, 8, 10);
    Word immediate = Utils::read_bit_range(opcode, 0, 7) << 2;

    Word pc_value = (read_register(REGISTER_PC) + 4) & (~1);
    Word sp_value = read_register(REGISTER_SP);
    Word source_value = sp ? sp_value : pc_value;

    CpuALU alu;
    Word result = OpcodeDataProcess::calculate_instruction(
        &alu, 
        OpcodeDataProcess::ADD, 
        source_value,
        immediate, 
        cpsr.c
    );

    write_register(destination_register, result);
}
