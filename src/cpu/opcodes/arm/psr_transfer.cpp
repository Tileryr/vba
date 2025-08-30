#include "psr_transfer.h"
#include "src/utils.h"

OpcodePsrTransfer::OpcodePsrTransfer(Word opcode)
{
    immediate_operand = Utils::read_bit(opcode, 25);
    psr = Utils::read_bit(opcode, 22); 
    is_msr_instruction = Utils::read_bit(opcode, 21);
    write_flag_bits = Utils::read_bit(opcode, 19);
    write_control_bits = Utils::read_bit(opcode, 16);
    register_destination = Utils::read_bit_range(opcode, 12, 15);
    immediate_rotate = Utils::read_bit_range(opcode, 8, 11);
    immediate_value = Utils::read_bit_range(opcode, 0, 7);
    register_source = Utils::read_bit_range(opcode, 0, 3);
}