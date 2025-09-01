#include "src/utils.h"
#include "src/cpu/opcodes/arm/branch.h"

OpcodeBranch::OpcodeBranch(Word opcode) {
    offset = Utils::read_bit_range(opcode, 0, 23);
    link_bit = Utils::read_bit(opcode, 24);
}

void OpcodeBranch::branch(ARM7TDMI * cpu, Word pc, Word offset_immediate, Byte bits_in_offset) {
    Word offset = Utils::sign_extend(offset_immediate, bits_in_offset);
    Word new_address = pc + offset;
    cpu->write_register(REGISTER_PC, new_address);
}