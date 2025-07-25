#ifndef UTIL_INCLUDED
#define UTIL_INCLUDED

#include "./cpu/cpu_types.h"
#include <sys/types.h>

enum Endian {
    ENDIAN_BIG = true,
    ENDIAN_LITTLE = false
};

typedef struct Utils {
    static void write_bit(Word * number, unsigned int bit_number, bool bit_value);
    static bool read_bit(u_int64_t number, unsigned int bit_number);

    static void write_bit_range(Word * number, unsigned int bit_range_start, unsigned int bit_range_end, unsigned int bit_value);
    static int read_bit_range(u_int64_t number, unsigned int bit_range_start, unsigned int bit_range_end);

    static Word logical_right_shift(int number, unsigned int shift_amount, unsigned int bit_size);
    static Word logical_left_shift(int number, unsigned int shift_amount, unsigned int bit_size);
    static int32_t arithmetic_right_shift(int number, unsigned int shift_amount);
    static Word rotate_right(unsigned int number, unsigned int rotate_amount, unsigned int bit_size);

    static int sign_extend(int base_number, int bits_in_base);

    static Word make_word(Byte byte_1, Byte byte_2, Byte byte_3, Byte byte_4);
    static Word current_word_at_memory(Byte * memory, Endian endian_type);

} Utils;

#endif