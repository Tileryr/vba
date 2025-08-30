#ifndef MEMORY_INCLUDED
#define MEMORY_INCLUDED

#include "src/cpu/cpu_types.h"

typedef struct Memory {
    Memory(Byte * memory);
    Memory(Word memory_size);
    ~Memory();
    
    Byte * memory;

    Word read_word_from_memory(Word address);
    HalfWord read_halfword_from_memory(Word address);
    Byte read_from_memory(Word address);

    void write_word_to_memory(Word address, Word value);
    void write_halfword_to_memory(Word address, HalfWord value);
    void write_to_memory(Word address, Byte value);

    Byte * memory_region(Word address);

    static Word read_word_from_memory(Byte * memory, Word address);
    static HalfWord read_halfword_from_memory(Byte * memory, Word address);
    static Byte read_from_memory(Byte * memory, Word address);
} Memory;

#endif