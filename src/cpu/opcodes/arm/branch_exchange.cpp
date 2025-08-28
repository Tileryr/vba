#include "src/cpu/opcodes/arm/branch_exchange.h"
#include "src/utils.h"

OpcodeBranchExchange::OpcodeBranchExchange() {}
OpcodeBranchExchange::OpcodeBranchExchange(Word opcode) {
    register_number = Utils::read_bit_range(opcode, 0, 3);
}

void OpcodeBranchExchange::build(OpcodeBranchExchange * target, Word new_register_number) {
    target->register_number = new_register_number;
}

void OpcodeBranchExchange::run(ARM7TDMI * cpu) {
    Word register_value = cpu->read_register(register_number);
    bool bit_one = Utils::read_bit(register_value, 0);

    if (bit_one)
    {
        cpu->cpsr.t = STATE_THUMB;
        cpu->write_register(REGISTER_PC, register_value - 1);
    } else 
    {
        cpu->cpsr.t = STATE_ARM;
        cpu->write_register(REGISTER_PC, register_value);
    }
}