#include "cpu.h"
#include "opcode_type.h"
#include "cpu_types.h"

OpcodeType ARM7TDMI::decode_opcode(Word opcode)
{
    if (Utils::read_bit_range(&opcode, 25, 27) == 0b101)
    {
        return BRANCH;
    }

}
