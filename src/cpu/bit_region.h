#ifndef BIT_REGION_INCLUDED
#define BIT_REGION_INCLUDED

#include "src/memory.h"
#include "src/cpu/cpu_types.h"

typedef struct BitRegion {
    Byte * memory_region;
    Byte start_bit;
    Byte end_bit;

    BitRegion(Byte * memory_region, Byte bit);
    BitRegion(Byte * memory_region, Byte start_bit, Byte end_bit);

    void set(Word value);
    Word get();
} BitRegion;

#endif
