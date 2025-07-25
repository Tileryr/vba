#include "cpu.h"
#include "opcode_type.h"
#include "cpu_types.h"

OpcodeType ARM7TDMI::decode_opcode_arm(Word opcode)
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
    if (Utils::read_bit_range(opcode, 26, 27) == 0b00)
    {
        return ALU;
    }
    if (Utils::read_bit_range(opcode, 22, 27) == 0b000000
    &&  Utils::read_bit_range(opcode, 4,  7 ) == 0b1001)
    {
        return MULTIPLY_LONG;
    }
    if (Utils::read_bit_range(opcode, 23, 27) == 0b00001
    &&  Utils::read_bit_range(opcode, 4,  7 ) == 0b1001)
    {
        return MULTIPLY_LONG;
    }
}
