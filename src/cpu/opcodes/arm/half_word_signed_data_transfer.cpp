#include "half_word_signed_data_transfer.h"
#include "src/utils.h"
OpcodeHalfWordSignedDataTransfer::OpcodeHalfWordSignedDataTransfer(Word opcode) : DataTransfer(opcode) {
    i = Utils::read_bit(opcode, 22);
    s = Utils::read_bit(opcode, 6);
    h = Utils::read_bit(opcode, 5);
    immediate_high_nibble = Utils::read_bit_range(opcode, 8, 11);
    immediate_low_nibble = Utils::read_bit_range(opcode, 0, 3);
}