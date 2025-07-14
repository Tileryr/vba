#include "cpu.h"
#include "opcode_type.h"
#include "cpu_types.h"

void ARM7TDMI::opcode_branch(Word opcode)
{
    Word pc_with_prefetch_offset = read_register(REGISTER_PC) + 8;
    int32_t offset = Utils::read_bit_range(&opcode, 0, 23);
    bool link_bit = Utils::read_bit(&opcode, 24);

    offset = offset << 2;
    offset = Utils::sign_extend(offset, 26);

    if (link_bit)
    {
        Word return_address = pc_with_prefetch_offset - 4;
        write_register(REGISTER_LS, return_address);
    }

    Word new_address = pc_with_prefetch_offset + offset;
    write_register(REGISTER_PC, new_address);
}

void ARM7TDMI::opcode_branch_exchange(Word opcode)
{
    Word register_number = Utils::read_bit_range(&opcode, 0, 3);
    Word register_value = read_register(register_number);
    bool bit_one = Utils::read_bit(&register_number, 0);

    if (bit_one)
    {
        cpsr.t = STATE_THUMB;
        write_register(REGISTER_PC, register_value - 1);
    } else 
    {
        cpsr.t = STATE_ARM;
        write_register(REGISTER_PC, register_value);
    }
}

void ARM7TDMI::opcode_software_interrupt(Word opcode)
{
    run_exception(EXCEPTION_SOFTWARE_INTERRUPT);
}

void ARM7TDMI::opcode_undefined_intruction(Word opcode)
{
    run_exception(EXCEPTION_UNDEFINED);
}

