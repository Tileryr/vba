#include "SDL3/SDL.h"
#include "src/cpu/opcodes/dissassembler.h"

std::string dissassemble_opcode_arm(ArmOpcodeType opcode_type) {
    switch (opcode_type) {
        case BRANCH:
            return "branch";
            break;
        case BX:
            return "BX";
            break;
        case SWI:
            return "SWI";
            break;
        case UNDEFINED:
            return "UNDEFINED";
            break;
        case ALU:
            return "alu";
            break;
        case MULTIPLY:
            return "multiply";
            break;
        case MULTIPLY_LONG:
            return "multiply_long";
            break;
        case PSR_TRANSFER:
            return "psr_transfer";
            break;
        case SINGLE_DATA_TRANSFER:
            return "single_data_transfer";
            break;
        case HALF_WORD_SIGNED_DATA_TRANSFER:
            return "half_word_data_transfer";
            break;
        case BLOCK_DATA_TRANSFER:
            return "block_data_transfer";
            break;
        case SWAP:
            return "swap";
            break;
    }
    
    SDL_assert(false);
    return "ERROR";
}

std::string dissassemble_opcode_thumb(ThumbOpcodeType opcode_type) {
    switch (opcode_type) {
        case MOVE_SHIFTED_REGISTER:
            return "MOVE_SHIFTED_REGISTER";
        case ADD_SUBTRACT:
            return "ADD_SUBTRACT";
        case MOVE_COMPARE_ADD_SUBTRACT_IMMEDIATE:
            return "MOVE_COMPARE_ADD_SUBTRACT_IMMEDIATE";
        case ALU_OPERATION:
            return "ALU_OPERATION";
        case HI_REGISTER_OPERATIONS_BRANCH_EXCHANGE:
            return "HI_REGISTER_OPERATIONS_BRANCH_EXCHANGE";
        case PC_RELATIVE_LOAD:
            return "PC_RELATIVE_LOAD";
        case LOAD_STORE_REGISTER_OFFSET:
            return "LOAD_STORE_REGISTER_OFFSET";
        case LOAD_STORE_SIGN_EXTENDED_BYTE_HALFWORD:
            return "LOAD_STORE_SIGN_EXTENDED_BYTE_HALFWORD";
        case LOAD_STORE_IMMEDIATE_OFFSET:
            return "LOAD_STORE_IMMEDIATE_OFFSET";
        case LOAD_STORE_HALFWORD:
            return "LOAD_STORE_HALFWORD";
        case SP_RELATIVE_LOAD_STORE:
            return "SP_RELATIVE_LOAD_STORE";
        case LOAD_ADDRESS:
            return "LOAD_ADDRESS";
        case ADD_OFFSET_TO_STACK_POINTER:
            return "ADD_OFFSET_TO_STACK_POINTER";
        case PUSH_POP_REGISTERS:
            return "PUSH_POP_REGISTERS";
        case MULTIPLE_LOAD_STORE:
            return "MULTIPLE_LOAD_STORE";
        case CONDITIONAL_BRANCH:
            return "CONDITIONAL_BRANCH";
        case SOFTWARE_INTERRUPT:
            return "SOFTWARE_INTERRUPT";
        case UNCONDITIONAL_BRANCH:
            return "UNCONDITIONAL_BRANCH";
        case LONG_BRANCH_WITH_LINK:
            return "LONG_BRANCH_WITH_LINK";
    }

    SDL_assert(false);
    return "ERROR";
}