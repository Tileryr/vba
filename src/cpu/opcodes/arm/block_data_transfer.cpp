#include "block_data_transfer.h"
#include "src/utils.h"

OpcodeBlockDataTransfer::OpcodeBlockDataTransfer(Word opcode) {
    p = Utils::read_bit(opcode, 24);
    u = Utils::read_bit(opcode, 23);
    s = Utils::read_bit(opcode, 22);
    w = Utils::read_bit(opcode, 21);
    l = Utils::read_bit(opcode, 20);

    base_register = Utils::read_bit_range(opcode, 16, 19);
    register_list = Utils::read_bit_range(opcode, 0,  15);
}
