#include <SDL3/SDL.h>

#include "src/cpu/cpu.h"
#include "src/cpu/opcodes/opcode_types.h"
#include "src/cpu/cpu_types.h"

ArmOpcodeType ARM7TDMI::decode_opcode_arm(Word opcode)
{
    if (Utils::read_bit_range(opcode, 25, 27) == 0b101)
    {
        return BRANCH;
    }
    if (Utils::read_bit_range(opcode, 4, 27) == 0b0001'0010'1111'1111'1111'0001)
    {
        return BX;
    }
    if (Utils::read_bit_range(opcode, 24, 27) == 0b1111)
    {
        return SWI;
    }
    if (Utils::read_bit_range(opcode, 25, 27) == 0b011
    &&  Utils::read_bit(opcode, 4) == 0b1)
    {
        return UNDEFINED;
    }
    if (Utils::read_bit_range(opcode, 23, 27) == 0b00010 &&
        Utils::read_bit_range(opcode, 4,  11) == 0b00001001 &&
        Utils::read_bit_range(opcode, 20, 21) == 0b00) {
        return SWAP;
    }
    if (Utils::read_bit_range(opcode, 26, 27) == 0b00
    &&  Utils::read_bit_range(opcode, 23, 24) == 0b10
    &&  Utils::read_bit(opcode, 20) == 0b0
    ) {
        return PSR_TRANSFER;
    }
    if (Utils::read_bit_range(opcode, 22, 27) == 0b000000
    &&  Utils::read_bit_range(opcode, 4,  7 ) == 0b1001)
    {
        return MULTIPLY;
    }
    if (Utils::read_bit_range(opcode, 23, 27) == 0b00001
    &&  Utils::read_bit_range(opcode, 4,  7 ) == 0b1001)
    {
        return MULTIPLY_LONG;
    }
    if (Utils::read_bit_range(opcode, 25, 27) == 0b000 && Utils::read_bit(opcode, 7) && Utils::read_bit(opcode, 4)) {
        return HALF_WORD_SIGNED_DATA_TRANSFER;
    }
    if (Utils::read_bit_range(opcode, 26, 27) == 0b00)
    {
        return ALU;
    }
    if (Utils::read_bit_range(opcode, 26, 27) == 0b01) {
        return SINGLE_DATA_TRANSFER;
    }
    if (Utils::read_bit_range(opcode, 25, 27) == 0b100) {
        return BLOCK_DATA_TRANSFER;
    }
    
    SDL_assert(false);
    return ALU;
}

ThumbOpcodeType ARM7TDMI::decode_opcode_thumb(HalfWord opcode) {
    auto get_bitregion = [opcode](Word region_start, Word region_end) {
        return Utils::read_bit_range(opcode, region_start, region_end);
    };
    auto get_bit = [opcode](Word bit) {
        return Utils::read_bit(opcode, bit);
    };
    if (get_bitregion(11, 15) == 0b00011) {
        return ADD_SUBTRACT;
    }
    if (get_bitregion(13, 15) == 0b000) {
        return MOVE_SHIFTED_REGISTER;
    }
    if (get_bitregion(13, 15) == 0b001) {
        return MOVE_COMPARE_ADD_SUBTRACT_IMMEDIATE;
    }
    if (get_bitregion(10, 15) == 0b010000) {
        return ALU_OPERATION;
    }
    if (get_bitregion(10, 15) == 0b010001) {
        return HI_REGISTER_OPERATIONS_BRANCH_EXCHANGE;
    }
    if (get_bitregion(11, 15) == 0b01001) {
        return PC_RELATIVE_LOAD;
    }
    if (get_bitregion(12, 15) == 0b0101 && get_bit(9) == 0b0) {
        return LOAD_STORE_REGISTER_OFFSET;
    }
    if (get_bitregion(12, 15) == 0b0101 && get_bit(9) == 0b1) {
        return LOAD_STORE_SIGN_EXTENDED_BYTE_HALFWORD;
    }
    if (get_bitregion(13, 15) == 0b011) {
        return LOAD_STORE_IMMEDIATE_OFFSET;
    }
    if (get_bitregion(12, 15) == 0b1000) {
        return LOAD_STORE_HALFWORD;
    }
    if (get_bitregion(12, 15) == 0b1001) {
        return SP_RELATIVE_LOAD_STORE;
    }
    if (get_bitregion(12, 15) == 0b1010) {
        return LOAD_ADDRESS;
    }
    if (get_bitregion(8,  15) == 0b10110000) {
        return ADD_OFFSET_TO_STACK_POINTER;
    }
    if (get_bitregion(12, 15) == 0b1011 && get_bitregion(9, 10) == 0b10) {
        return PUSH_POP_REGISTERS;
    }
    if (get_bitregion(12, 15) == 0b1100) {
        return MULTIPLE_LOAD_STORE;
    }
    if (get_bitregion(12, 15) == 0b1101) {
        return BRANCH_CONDITIONAL;
    }
    if (get_bitregion(8,  15) == 0b11011111) {
        return SOFTWARE_INTERRUPT;
    }
    if (get_bitregion(11, 15) == 0b11100) {
        return BRANCH_UNCONDITIONAL;
    }
    if (get_bitregion(12, 15) == 0b1111) {
        return LONG_BRANCH_WITH_LINK;
    }

    SDL_assert(false);
    return ALU_OPERATION;
}