#include "SDL3/SDL.h"
#include "cpu.h"

std::string ARM7TDMI::dissassemble_opcode(Word opcode) {
    ArmOpcodeType opcode_type = decode_opcode_arm(opcode);
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
}