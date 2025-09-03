#include "src/cpu/cpu.h"

#include "src/cpu/opcodes/arm/branch.h"
#include "src/cpu/opcodes/arm/data_processing.h"
#include "src/cpu/opcodes/arm/multiply.h"
#include "src/cpu/opcodes/arm/single_data_transfer.h"
#include "src/cpu/opcodes/arm/half_word_signed_data_transfer.h"
#include "src/cpu/opcodes/arm/block_data_transfer.h"
#include "src/cpu/opcodes/arm/branch_exchange.h"

#include <SDL3/SDL.h>
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

void ARM7TDMI::thumb_opcode_hi_register_operations_branch_exchange(HalfWord opcode) {
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

void ARM7TDMI::thumb_opcode_pc_relative_load(HalfWord opcode) {
    Byte destination_register = Utils::read_bit_range(opcode, 8, 10);
    Word word_8 = Utils::read_bit_range(opcode, 0, 7);

    Word immediate = word_8 << 2;

    OpcodeSingleDataTransferBuilder(true, REGISTER_PC, destination_register)
    .set_flags(true, true, false, false)
    .set_offset_immediate(immediate)
    .get_product().run(this);
}

void ARM7TDMI::thumb_opcode_load_store_register_offset(HalfWord opcode) {
    bool load = Utils::read_bit(opcode, 11);
    bool byte = Utils::read_bit(opcode, 10);

    Byte offset_register = Utils::read_bit_range(opcode, 6, 8);
    Byte base_register = Utils::read_bit_range(opcode, 3, 5);
    Byte source_destination_register = Utils::read_bit_range(opcode, 0, 2);

    OpcodeSingleDataTransferBuilder(load, base_register, source_destination_register)
    .set_flags(true, true, byte, false)
    .set_offset_register(offset_register, 0, 0)
    .get_product().run(this);
}

void ARM7TDMI::thumb_opcode_load_store_sign_extended_byte_halfword(HalfWord opcode) {
    bool h = Utils::read_bit(opcode, 11);
    bool sign_extend = Utils::read_bit(opcode, 10);

    Byte offset_register = Utils::read_bit_range(opcode, 6, 8);
    Byte base_register = Utils::read_bit_range(opcode, 3, 5);
    Byte source_destination_register = Utils::read_bit_range(opcode, 0, 2);

    bool load;
    if (h == 0 && sign_extend == 0) {
        load = false;
    } else {
        load = true;
    }

    OpcodeHalfWordSignedDataTransfer::DataType data_type;
    if (h == 0 && sign_extend == 0) {
        data_type = OpcodeHalfWordSignedDataTransfer::UNSIGNED_HALFWORD;
    } else {
        Byte sh = (sign_extend << 1) | h;
        data_type = (OpcodeHalfWordSignedDataTransfer::DataType) sh;
    }

    OpcodeHalfWordSignedDataTransferBuilder(load, base_register, source_destination_register)
    .set_datatype(data_type)
    .set_flags(true, true, false)
    .set_offset_register(offset_register)
    .get_product().run(this);
}

void ARM7TDMI::thumb_opcode_load_store_immediate_offset(HalfWord opcode) {
    bool byte = Utils::read_bit(opcode, 12);
    bool load = Utils::read_bit(opcode, 11);

    Byte offset_value = Utils::read_bit_range(opcode, 6, 10);
    Byte base_register = Utils::read_bit_range(opcode, 3, 5);
    Byte source_destination_register = Utils::read_bit_range(opcode, 0, 2);

    Byte immediate = byte ? offset_value : offset_value << 2;

    OpcodeSingleDataTransferBuilder(load, base_register, source_destination_register)
    .set_flags(true, true, byte, false)
    .set_offset_immediate(immediate)
    .get_product().run(this);
}

void ARM7TDMI::thumb_opcode_load_store_halfword(HalfWord opcode) {
    bool load = Utils::read_bit(opcode, 11);

    Byte offset_value = Utils::read_bit_range(opcode, 6, 10);
    Byte base_register = Utils::read_bit_range(opcode, 3, 5);
    Byte source_destination_register = Utils::read_bit_range(opcode, 0, 2);

    Byte immediate = offset_value << 1;

    OpcodeHalfWordSignedDataTransferBuilder(load, base_register, source_destination_register)
    .set_datatype(OpcodeHalfWordSignedDataTransfer::UNSIGNED_HALFWORD)
    .set_flags(true, true, false)
    .set_offset_immediate(immediate)
    .get_product().run(this);
}

void ARM7TDMI::thumb_opcode_sp_relative_load_store(HalfWord opcode) {
    bool load = Utils::read_bit(opcode, 11);
    Byte destination_register = Utils::read_bit_range(opcode, 8, 10);
    Word word_8 = Utils::read_bit_range(opcode, 0, 7);

    Word immediate = word_8 << 2;

    OpcodeSingleDataTransferBuilder(load, REGISTER_SP, destination_register)
    .set_flags(true, true, false, false)
    .set_offset_immediate(immediate)
    .get_product().run(this);
}

void ARM7TDMI::thumb_opcode_load_address(HalfWord opcode) {
    bool sp = Utils::read_bit(opcode, 11);
    Byte destination_register = Utils::read_bit_range(opcode, 8, 10);
    Word word_8 = Utils::read_bit_range(opcode, 0, 7);

    Byte source_register = sp ? REGISTER_SP : REGISTER_PC;
    Word immediate = word_8 << 2;
    
    OpcodeDataProcessingBuilder(OpcodeDataProcess::ADD, false)
    .set_destination_register(destination_register)
    .set_source_register(source_register)
    .set_immediate_op2(immediate, 0)
    .get_product().run(this);

    if (source_register == REGISTER_PC) {
        Word wrote_value = read_register(destination_register);
        write_register(destination_register, wrote_value & (~0b10));
    }
}

void ARM7TDMI::thumb_opcode_add_offset_to_stack_pointer(HalfWord opcode) {
    bool sign = Utils::read_bit(opcode, 7);
    Word s_word_7 = Utils::read_bit_range(opcode, 0, 6);

    Word immediate = s_word_7 << 2;

    OpcodeDataProcess::InstructionType instruction_type;
    if (sign == 0) {
        instruction_type = OpcodeDataProcess::ADD;
    } else {
        instruction_type = OpcodeDataProcess::SUB;
    }

    OpcodeDataProcessingBuilder(instruction_type, false)
    .set_destination_register(REGISTER_SP)
    .set_source_register(REGISTER_SP)
    .set_immediate_op2(immediate, 0)
    .get_product().run(this);
}

void ARM7TDMI::thumb_opcode_push_pop_registers(HalfWord opcode) {
    bool load = Utils::read_bit(opcode, 11);
    bool pc_lr_bit = Utils::read_bit(opcode, 8);
    Byte r_list = Utils::read_bit_range(opcode, 0, 7);

    bool up  = load ? true : false;
    bool pre = load ? false : true;

    HalfWord register_list = r_list;

    if (pc_lr_bit) {
        if (load) {
            register_list = register_list | (1 << REGISTER_PC);
        } else {
            register_list = register_list | (1 << REGISTER_LR);
        }
    }

    OpcodeBlockDataTransferBuilder(load, REGISTER_SP)
    .set_flags(pre, up, false, true)
    .set_register_list(register_list)
    .get_product().run(this);
}

void ARM7TDMI::thumb_opcode_multiple_load_store(HalfWord opcode) {
    bool load = Utils::read_bit(opcode, 11);
    Byte base_register = Utils::read_bit_range(opcode, 8, 10);
    Byte register_list = Utils::read_bit_range(opcode, 0, 7);

    OpcodeBlockDataTransferBuilder(load, base_register)
    .set_flags(false, true, false, true)
    .set_register_list(register_list)
    .get_product().run(this);
}   

void ARM7TDMI::thumb_opcode_conditional_branch(HalfWord opcode) {
    Byte condition = Utils::read_bit_range(opcode, 8, 11);
    Word s_offset_8 = Utils::read_bit_range(opcode, 0, 7);

    if (condition_field(condition) == false) {
        return;
    }

    Word pc = read_register(REGISTER_PC) + 4;
    Word immediate = s_offset_8 << 1;

    OpcodeBranch::branch(this, pc, immediate, 9);
}

void ARM7TDMI::thumb_opcode_software_interrupt(HalfWord opcode) {
    run_exception(EXCEPTION_SOFTWARE_INTERRUPT);
}

void ARM7TDMI::thumb_opcode_unconditional_branch(HalfWord opcode) {
    HalfWord offset_11 = Utils::read_bit_range(opcode, 0, 10);
    Word pc = read_register(REGISTER_PC) + 4;
    Word immediate = offset_11 << 1;

    OpcodeBranch::branch(this, pc, immediate, 12);
}  

void ARM7TDMI::thumb_opcode_long_branch_with_link(HalfWord opcode) {
    bool low_offset = Utils::read_bit(opcode, 11);
    Word offset = Utils::read_bit_range(opcode, 0, 10);

    if (low_offset == 0) {
        Word pc = read_register(REGISTER_PC) + 4;
        
        Word signed_high_offset = Utils::sign_extend((offset << 12), 23);
        write_register(REGISTER_LR, signed_high_offset + pc);
    } else {
        Word branch_offset = (offset << 1) + read_register(REGISTER_LR);
        Word following_address = read_register(REGISTER_PC) + 2;

        write_register(REGISTER_LR, following_address | 1);
        write_register(REGISTER_PC, branch_offset);
    }
}
