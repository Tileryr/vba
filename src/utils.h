#include <sys/types.h>

typedef struct Utils {
    static void write_bit(unsigned int * number, unsigned int bit_number, bool bit_value);

    static bool read_bit(unsigned int * number, unsigned int bit_number);

    static void write_bit_range(unsigned int * number, unsigned int bit_range_start, unsigned int bit_range_end, unsigned int bit_value);


    static int read_bit_range(unsigned int * number, unsigned int bit_range_start, unsigned int bit_range_end);
} Utils;
