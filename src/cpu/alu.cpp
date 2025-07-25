#include <limits.h>
#include <stdarg.h>
#include <cstdio>
#include <stdint.h>

#include "cpu_types.h"
#include "alu.h"
#include "../utils.h"

Word ArithmaticLogicUnit::add(int total_addends, ...) 
{
    u_int64_t result = 0;
    va_list addends;

    va_start(addends, total_addends);
    for (int i = 0; i < total_addends; i++)
    {
        Word arg = va_arg(addends, Word);
        result += arg;
    }
    va_end(addends);

    if (result > UINT_MAX)
    
        carry_flag = true;
    else 
        carry_flag = false;

    // printf("%d \n", result);
    
    return result & UINT_MAX;
};

Word ArithmaticLogicUnit::subtract(int total_subtrahends, Word base, ...) 
{
    u_int64_t result = base;
    va_list addends;

    va_start(addends, base);
    for (int i = 0; i < total_subtrahends; i++)
    {
        Word arg = va_arg(addends, Word);
        result -= arg;
    }
    va_end(addends);

    if (result > UINT_MAX)
        carry_flag = false;
    else 
        carry_flag = true;

    return result & UINT_MAX;
};

Word    ArithmaticLogicUnit::logical_left_shift(Word number, unsigned int shift_amount)
{
    if (shift_amount < 32) {
        carry_flag = Utils::read_bit(number, (31 - shift_amount) + 1);
    } else if (shift_amount == 32) {
        carry_flag = Utils::read_bit(number, 0);
        return 0;
    } else if (shift_amount > 32) {
        carry_flag = 0;
        return 0;
    }
    
    return (number << shift_amount) & UINT32_MAX;
}
Word    ArithmaticLogicUnit::logical_right_shift(Word number, unsigned int shift_amount)
{
    
    if (shift_amount == 32) {
        carry_flag = Utils::read_bit(number, 31);
        return 0;
    } else if (shift_amount > 32) {
        carry_flag = 0;
        return 0;
    }

    Word unsigned_number = number & UINT32_MAX;
    carry_flag = Utils::read_bit(unsigned_number, shift_amount - 1);
    return (unsigned_number >> shift_amount);
}
int32_t ArithmaticLogicUnit::arithmetic_right_shift(int32_t number, unsigned int shift_amount)
{
    if (shift_amount >= 32) {
        bool bit_31 = Utils::read_bit(number, 31);
        carry_flag = bit_31;
        if (bit_31)
            return INT32_MAX;
        else
            return 0;
    }

    carry_flag = Utils::read_bit(number, shift_amount - 1);
    return (number >> shift_amount) & UINT32_MAX;
}
Word    ArithmaticLogicUnit::rotate_right(Word number, unsigned int rotate_amount)
{
    while (rotate_amount > 32)
    {
        rotate_amount -= 32;
    }
    
    if (rotate_amount == 32) {
        carry_flag = Utils::read_bit(number, 31);
        return number;
    }

    carry_flag = Utils::read_bit(number, rotate_amount - 1);
    
    Word right_shifted_number = number >> rotate_amount;
    Word rotated_bits = number << (32 - rotate_amount);
    Word result = right_shifted_number | rotated_bits;
    result = result & UINT32_MAX;
    return result;
}

Word ArithmaticLogicUnit::rotate_right_extended(Word number, bool cpsr_c)
{
    carry_flag = Utils::read_bit(number, 0);
    return (number >> 1) | (cpsr_c << 31);
}
