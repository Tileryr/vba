#include "./utils.h"
#include <cstdio>
#include <assert.h>

// template <class T>
// void Utils::write_bit(T number, unsigned int bit_number, bool bit_value) 
// {
//     if (bit_value == 0)
//         *number = *number & (~(1 << bit_number));
//     else
//         *number = *number | (1 << bit_number);
// }

bool Utils::read_bit(u_int64_t number, unsigned int bit_number) 
{
    return (number >> bit_number) & 1;
}

void Utils::write_bit_range(Word * number, unsigned int bit_range_start, unsigned int bit_range_end, unsigned int bit_value) 
{
    for (int i = 0; i < (bit_range_end - bit_range_start) + 1; i++)
    {
        write_bit(number, bit_range_start + i, read_bit(bit_value, i));
    }
}

int Utils::read_bit_range(u_int64_t number, unsigned int bit_range_start, unsigned int bit_range_end) 
{
    unsigned int mask = (1<<(bit_range_end-bit_range_start+1))-1;
    return (number>>bit_range_start)&mask;
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
