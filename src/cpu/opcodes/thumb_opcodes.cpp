#include "src/cpu/cpu.h"
#include "src/cpu/opcodes/arm/data_processing.h"
#include "src/cpu/opcodes/arm/multiply.h"
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

    OpcodeDataProcessingBuilder(OpcodeDataProcess::MOV, true)
    .set_destination_register(destination_register)
    .set_register_op2(source_register, shift_type)
    .set_register_op2_shift_immediate(offset)
    .get_product()
    .run(this);
}

void ARM7TDMI::thumb_opcode_add_subtract(HalfWord opcode) {
    bool immediate_flag = Utils::read_bit(opcode, 10);
    bool sub_opcode = Utils::read_bit(opcode, 9);

    Byte op2_register_immediate = Utils::read_bit_range(opcode, 6, 8);    
    Byte source_register = Utils::read_bit_range(opcode, 3, 5);
    Byte destination_register = Utils::read_bit_range(opcode, 0, 2);

    OpcodeDataProcess::InstructionType instruction_type;

    switch (sub_opcode)
    {
        case 0:
            instruction_type = OpcodeDataProcess::ADD;
            break;
        case 1:
            instruction_type = OpcodeDataProcess::SUB;
            break;
    }

    OpcodeDataProcessingBuilder builder = OpcodeDataProcessingBuilder(instruction_type, true)
    .set_destination_register(destination_register)
    .set_source_register(source_register);

    if (immediate_flag) {
        builder.set_immediate_op2(op2_register_immediate, 0);
    } else {
        builder.set_register_op2(op2_register_immediate, 0)
        .set_register_op2_shift_immediate(0);
    }
    
    builder.get_product().run(this);
}

void ARM7TDMI::thumb_opcode_move_compare_add_subtract(HalfWord opcode) {
    Byte opcode_instruction_type = Utils::read_bit_range(opcode, 11, 12);
    Byte source_destination_register = Utils::read_bit_range(opcode, 8, 10);
    Byte immediate = Utils::read_bit_range(opcode, 0, 7);

    OpcodeDataProcess::InstructionType instruction_type;

    switch (opcode_instruction_type)
    {
        case 0: instruction_type = OpcodeDataProcess::MOV; break;
        case 1: instruction_type = OpcodeDataProcess::CMP; break;
        case 2: instruction_type = OpcodeDataProcess::ADD; break;
        case 3: instruction_type = OpcodeDataProcess::SUB; break;
    }

    OpcodeDataProcessingBuilder(instruction_type, true)
    .set_destination_register(source_destination_register)
    .set_source_register(source_destination_register)
    .set_immediate_op2(immediate, 0)
    .get_product()
    .run(this);
}

void ARM7TDMI::thumb_opcode_alu_operations(HalfWord opcode) {
    Byte sub_opcode = Utils::read_bit_range(opcode, 6, 9);
    Byte source_register_2 = Utils::read_bit_range(opcode, 3, 5);
    Byte source_destination_register = Utils::read_bit_range(opcode, 0, 2);

    OpcodeDataProcess::InstructionType instruction_type;
    
    bool multiply = false;
    bool neg = false;

    bool mov_shift = false;
    OpcodeDataProcess::BitShiftType shift_type;

    switch (sub_opcode)
    {
        case 0x0: instruction_type = OpcodeDataProcess::AND; break;
        case 0x1: instruction_type = OpcodeDataProcess::EOR; break;
        case 0x2: mov_shift = true; shift_type = OpcodeDataProcess::LSL; break; // LSL
        case 0x3: mov_shift = true; shift_type = OpcodeDataProcess::LSR; break; // LSR
        case 0x4: mov_shift = true; shift_type = OpcodeDataProcess::ASR; break; // ASR
        case 0x5: instruction_type = OpcodeDataProcess::ADC; break;
        case 0x6: instruction_type = OpcodeDataProcess::SBC; break;
        case 0x7: mov_shift = true; shift_type = OpcodeDataProcess::ROR; break; // ROR

        case 0x8: instruction_type = OpcodeDataProcess::TST; break;
        case 0x9: neg = true; break; // NEG
        case 0xa: instruction_type = OpcodeDataProcess::CMP; break;
        case 0xb: instruction_type = OpcodeDataProcess::CMN; break;
        case 0xc: instruction_type = OpcodeDataProcess::ORR; break;
        case 0xd: multiply = true; break; // MULTIPLY
        case 0xe: instruction_type = OpcodeDataProcess::BIC; break;
        case 0xf: instruction_type = OpcodeDataProcess::MVN; break;
    }

    if (multiply) {
        OpcodeMultiplyBuilder(
            source_destination_register, 
            source_destination_register, 
            source_register_2, 
            true
        ).get_product().run(this);
    } else if (mov_shift) {
        OpcodeDataProcessingBuilder(OpcodeDataProcess::MOV, true)
        .set_destination_register(source_destination_register)
        .set_register_op2(source_destination_register, shift_type)
        .set_register_op2_shift_register(source_register_2)
        .get_product().run(this);
    } else if (neg) {
        OpcodeDataProcessingBuilder(OpcodeDataProcess::RSB, true)
        .set_destination_register(source_destination_register)
        .set_source_register(source_register_2)
        .set_immediate_op2(0, 0)
        .get_product().run(this);
    } else {
        OpcodeDataProcessingBuilder(instruction_type, true)
        .set_destination_register(source_destination_register)
        .set_source_register(source_destination_register)
        .set_register_op2(source_register_2, 0)
        .set_register_op2_shift_immediate(0)
        .get_product().run(this);
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

    OpcodeDataProcessingBuilder(instruction_type, instruction_type == OpcodeDataProcess::CMP ? true : false)
    .set_destination_register(target_destination_register)
    .set_source_register(target_destination_register)
    .set_register_op2(target_source_register, 0)
    .set_register_op2_shift_immediate(0)
    .get_product().run(this);
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
