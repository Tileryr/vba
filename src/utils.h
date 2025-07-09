#include <sys/types.h>

typedef struct Utils {
    static void write_bit(unsigned int * number, unsigned int bit_number, bool bit_value) 
    {
        if (bit_value == 0)
            *number = *number & (~(1 << bit_number));
        else
            *number = *number | (1 << bit_number);
    }

    static bool read_bit(unsigned int * number, unsigned int bit_number) 
    {
        return (*number >> bit_number) & 1;
    }

    static void write_bit_range(unsigned int * number, unsigned int bit_range_start, unsigned int bit_range_end, unsigned int bit_value) 
    {
        for (int i = 0; i < bit_range_end + 1; i++)
        {
            write_bit(number, bit_range_start + i, read_bit(&bit_value, i));
        }
    }

    static int read_bit_range(unsigned int * number, unsigned int bit_range_start, unsigned int bit_range_end) 
    {
        int result = 0;
        for (int i = 0; i < bit_range_end + 1; i++)
        {
            result += read_bit(number, bit_range_start + i) << i;
        }
        return result;
    }
} Utils;
