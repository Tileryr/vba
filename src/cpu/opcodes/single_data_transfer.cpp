#include "single_data_transfer.h"
#include "../../utils.h"

OpcodeSingleDataTransfer::OpcodeSingleDataTransfer(Word opcode)
{
    i = Utils::read_bit(opcode, 25);
    p = Utils::read_bit(opcode, 24);
    u = Utils::read_bit(opcode, 23);
    b = Utils::read_bit(opcode, 22);
    w = Utils::read_bit(opcode, 21);
    l = Utils::read_bit(opcode, 20);
    source_destination_register = Utils::read_bit_range(opcode, 16, 19);
    base_register = Utils::read_bit_range(opcode, 12, 15);
    offset_immediate = Utils::read_bit_range(opcode, 0, 11);
    register_shift_amount = Utils::read_bit_range(opcode, 7, 11);
    register_shift_type = Utils::read_bit_range(opcode, 5, 6);
    offset_register = Utils::read_bit_range(opcode, 0, 3);
}