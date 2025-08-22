#include "single_data_transfer.h"
#include "../../utils.h"

OpcodeSingleDataTransfer::OpcodeSingleDataTransfer(Word opcode) : DataTransfer(opcode) {
    i = Utils::read_bit(opcode, 25);


    b = Utils::read_bit(opcode, 22);

    
    offset_immediate = Utils::read_bit_range(opcode, 0, 11);
    register_shift_amount = Utils::read_bit_range(opcode, 7, 11);
    register_shift_type = Utils::read_bit_range(opcode, 5, 6);
    
}