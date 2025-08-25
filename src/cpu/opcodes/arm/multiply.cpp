#include "multiply.h"
#include "src/utils.h"

OpcodeMultiply::OpcodeMultiply(Word opcode)
{
    accumulate = Utils::read_bit(opcode, 21);
    set_condition_codes = Utils::read_bit(opcode, 20);
    register_destination = Utils::read_bit_range(opcode, 16, 19);
    rn = Utils::read_bit_range(opcode, 12, 15);
    rs = Utils::read_bit_range(opcode, 8, 11);
    rm = Utils::read_bit_range(opcode, 0, 3);
}