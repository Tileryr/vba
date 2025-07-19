#include "./utils.h"
#include <cstdio>
#include <assert.h>

void Utils::write_bit(Word * number, unsigned int bit_number, bool bit_value) 
{
    if (bit_value == 0)
        *number = *number & (~(1 << bit_number));
    else
        *number = *number | (1 << bit_number);
}

bool Utils::read_bit(u_int64_t number, unsigned int bit_number) 
{
    return (number >> bit_number) & 1;
}

void Utils::write_bit_range(Word * number, unsigned int bit_range_start, unsigned int bit_range_end, unsigned int bit_value) 
{
    for (int i = 0; i < bit_range_end + 1; i++)
    {
        write_bit(number, bit_range_start + i, read_bit(bit_value, i));
    }
}

int Utils::read_bit_range(u_int64_t number, unsigned int bit_range_start, unsigned int bit_range_end) 
{
    int result = 0;
    for (int i = 0; i < bit_range_end + 1; i++)
    {
        result += read_bit(number, bit_range_start + i) << i;
    }
    return result;
}

int Utils::sign_extend(int base_number, int bits_in_base) {
    int mask = 1 << (bits_in_base - 1);
    return (base_number ^ mask) - mask;
};

int Utils::arithmetic_right_shift(int number, unsigned int shift_amount) {
    return number >> shift_amount;
}

unsigned int Utils::logical_right_shift(int number, unsigned int shift_amount, unsigned int bit_size) {
    unsigned int mask = ((1 << bit_size) - 1);
    unsigned int unsigned_number = number & mask;
    return (unsigned_number >> shift_amount);
}

unsigned int Utils::rotate_right(unsigned int number, unsigned int rotate_amount, unsigned int bit_size) {
    int right_shifted_number = number >> rotate_amount;
    int rotated_bits = number << (bit_size - rotate_amount);
    int result = right_shifted_number | rotated_bits;
    result = result & ((1 << bit_size) - 1);
    return result;
}

Word Utils::make_word(Byte byte_1, Byte byte_2, Byte byte_3, Byte byte_4)
{
    return 
      (byte_1 << 0) 
    | (byte_2 << 8)
    | (byte_3 << 16)
    | (byte_4 << 24);
}

Word Utils::current_word_at_memory(Byte * memory, Endian endian_type)
{
    if (endian_type == BIG_ENDIAN)
    {
        return make_word(memory[3], memory[2], memory[1], memory[0]);
    } else 
    if (endian_type == LITTLE_ENDIAN)
    {
        return make_word(memory[0], memory[1], memory[2], memory[3]);
    }

    assert(false);
}

