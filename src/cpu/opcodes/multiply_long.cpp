#include "multiply_long.h"
#include "../../utils.h"

OpcodeMultiplyLong::OpcodeMultiplyLong(Word opcode)
{
    sign = Utils::read_bit(opcode, 22);
    accumulate = Utils::read_bit(opcode, 21);
    set_condition_codes = Utils::read_bit(opcode, 20);
    register_destination_high = Utils::read_bit_range(opcode, 16, 19);
    register_destination_low = Utils::read_bit_range(opcode, 12, 15);
    rs = Utils::read_bit_range(opcode, 8, 11);
    rm = Utils::read_bit_range(opcode, 0, 3);
}