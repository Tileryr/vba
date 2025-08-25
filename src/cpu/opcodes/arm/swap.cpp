#include "src/cpu/opcodes/arm/swap.h"
#include "src/utils.h"

OpcodeSwap::OpcodeSwap(Word opcode) {
    b = Utils::read_bit(opcode, 22);
    base_register = Utils::read_bit_range(opcode, 16, 19);
    destination_register = Utils::read_bit_range(opcode, 12, 15);
    source_register = Utils::read_bit_range(opcode, 0, 3);
}