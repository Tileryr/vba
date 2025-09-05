#include <math.h>
#include "src/utils.h"
#include "src/cpu/bit_region.h"

BitRegion::BitRegion(Byte * memory_region, Byte start_bit, Byte end_bit) : 
memory_region(memory_region), start_bit(start_bit), end_bit(end_bit) 
{}

BitRegion::BitRegion(Byte * memory_region, Byte bit) : 
memory_region(memory_region), start_bit(bit), end_bit(bit) {}

void BitRegion::set(Word value) {
    Word current_value = Memory::read_word_from_memory(memory_region, 0);
    Utils::write_bit_range(&current_value, start_bit, end_bit, value);
    Memory::write_word_to_memory(memory_region, 0, current_value);
}

Word BitRegion::get() {
    Word current_value = Memory::read_word_from_memory(memory_region, 0);
    return Utils::read_bit_range(current_value, start_bit, end_bit);
}