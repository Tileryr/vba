#include "src/memory.h"

Memory::Memory(Byte * memory) : memory(memory) {}

Memory::Memory(Word memory_size) {
    memory = new Byte[memory_size];
}

Memory::~Memory() {
    delete[] memory;
}

Word Memory::read_word_from_memory(Word address) {
    return read_word_from_memory(memory, address);
}

HalfWord Memory::read_halfword_from_memory(Word address) {
    return read_halfword_from_memory(memory, address);
}

Byte Memory::read_from_memory(Word address) {
    return read_from_memory(memory, address);
}
    
void Memory::write_word_to_memory(Word address, Word value) {
    write_word_to_memory(memory, address, value);
}

void Memory::write_halfword_to_memory(Word address, HalfWord value) {
    write_halfword_to_memory(memory, address, value);
}

void Memory::write_to_memory(Word address, Byte value) {
    write_to_memory(memory, address, value);
}

Word Memory::read_word_from_memory(Byte * memory, Word address) {
    return (
      (memory[address] << 0) 
    | (memory[address + 1] << 8)
    | (memory[address + 2] << 16)
    | (memory[address + 3] << 24)
    );
}

HalfWord Memory::read_halfword_from_memory(Byte * memory, Word address) {
    return (
        (memory[address]     << 0) |
        (memory[address + 1] << 8)
    );
}

Byte Memory::read_from_memory(Byte * memory, Word address) {
    return memory[address];
}

void Memory::write_word_to_memory(Byte * memory, Word address, Word value) {
    memory[address + 0] = (value >> 0) & 0xFF;
    memory[address + 1] = (value >> 8) & 0xFF;
    memory[address + 2] = (value >> 16) & 0xFF;
    memory[address + 3] = (value >> 24) & 0xFF;
}

void Memory::write_halfword_to_memory(Byte * memory, Word address, HalfWord value) {
    memory[address + 0] = (value >> 0) & 0xFF;
    memory[address + 1] = (value >> 8) & 0xFF;
}

void Memory::write_to_memory(Byte * memory, Word address, Byte value) {
    memory[address] = value;
}

Byte * Memory::memory_region(Word address) {
    return &memory[address];
}

