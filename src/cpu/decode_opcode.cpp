#include "cpu.h"
#include "opcode_type.h"
#include "cpu_types.h"

OpcodeType ARM7TDMI::decode_opcode_arm(Word opcode)
{
    if (Utils::read_bit_range(&opcode, 25, 27) == 0b101)
    {
        return BRANCH;
    }
    if (Utils::read_bit_range(&opcode, 4, 27) == 0b0001'0010'1111'1111'1111'0001)
    {
        return BX;
    }
    if (Utils::read_bit_range(&opcode, 24, 27) == 0b1111)
    {
        return SWI;
    }
}
