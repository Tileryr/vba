#include "src/memory.h"
#include <SDL3/SDL.h>
#define SDL_Log 
Word Memory::read_word_from_memory(Word address) {
    return (
        (read_from_memory(address  ) << 0) 
    |   (read_from_memory(address+1) << 8)
    |   (read_from_memory(address+2) << 16)
    |   (read_from_memory(address+3) << 24)
    );
}

HalfWord Memory::read_halfword_from_memory(Word address) {
    return (
        (read_from_memory(address  ) << 0) 
    |   (read_from_memory(address+1) << 8)
    );
}

Byte Memory::read_from_memory(Word address) {
    Word address_space = address >> 24;
    Word address_main = address & 0x00FFFFFF;

    MemoryPointer memory_pointer = address_to_memory_pointer(address);
    if (memory_pointer.valid == true) {
        for (auto region : addressable_regions) {
            if (address >= region.base_address && address <= region.base_address+region.length) {
                return region.read(*memory_pointer.pointer);
            }
        }
        
        return *memory_pointer.pointer;
    }

    return 0x0;
}

void Memory::write_word_to_memory(Word address, Word value) {
    write_to_memory(address  , (value >> 0 ) & 0xFF); 
    write_to_memory(address+1, (value >> 8 ) & 0xFF); 
    write_to_memory(address+2, (value >> 16) & 0xFF); 
    write_to_memory(address+3, (value >> 24) & 0xFF); 
}

void Memory::write_halfword_to_memory(Word address, HalfWord value) {
    write_to_memory(address  , (value >> 0 ) & 0xFF); 
    write_to_memory(address+1, (value >> 8 ) & 0xFF); 
}

void Memory::write_to_memory(Word address, Byte value) {
    Word address_space = address >> 24;
    Word address_main = address & 0x00FFFFFF;

    MemoryPointer memory_pointer = address_to_memory_pointer(address);
    if (memory_pointer.valid == true) {
        for (auto region : addressable_regions) {
            if (address >= region.base_address && address <= region.base_address+region.length) {
                value = region.write(*memory_pointer.pointer, value);
            }
        }
        *memory_pointer.pointer = value;
    }
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

Memory::MemoryPointer Memory::address_to_memory_pointer(Word address) {
    Word address_space = address >> 24;
    Word address_main = address & 0x00FFFFFF;

    MemoryPointer memory_pointer;
    memory_pointer.valid = false;

    switch (address_space) {
        case 0x0:
            if (address_main < 0x4000) {
                SDL_Log("WEIRD ACCESS - BIOS, %0x", address);
            } else {
                SDL_Log("WEIRD ACCESS - UNUSED (0x0)");
            }
            return memory_pointer;
        case 0x1:
            SDL_Log("WEIRD ACCESS - UNUSED");
            return memory_pointer;
        case 0x2:
            memory_pointer.pointer = &wram_board[address_main % WRAM_BOARD_SIZE];
            memory_pointer.valid = true;
            return memory_pointer;
        case 0x3:
            memory_pointer.pointer = &wram_chip[address_main % WRAM_CHIP_SIZE];
            memory_pointer.valid = true;
            return memory_pointer;
        case 0x4:
            if (address_main < IO_REGISTERS_SIZE) {
                memory_pointer.pointer = &io_registers[address_main];
                memory_pointer.valid = true;
                return memory_pointer;
            }
            SDL_Log("WEIRD ACCESS - IO_AREA");
            return memory_pointer;
        case 0x5:
            memory_pointer.pointer = &palette_ram[address_main % PALETTE_RAM_SIZE];\
            memory_pointer.valid = true;
            return memory_pointer;
        case 0x6: {
            Word vram_address = address_main % 0x20000;
            if (vram_address < 0x10000) {
                memory_pointer.pointer = &vram[vram_address];
                memory_pointer.valid = true;
                return memory_pointer;
            } else {
                memory_pointer.pointer = &vram[0x10000 + (vram_address % 0x8000)];
                memory_pointer.valid = true;
                return memory_pointer;
            }
        }
        case 0x7:
            memory_pointer.pointer = &oam[address_main % OAM_SIZE];
            memory_pointer.valid = true;
            return memory_pointer;
    }

    if (address_space < 0xE) {
        memory_pointer.pointer = &game_pak_rom[address - (0x08 << 24)];
        memory_pointer.valid = true;
        return memory_pointer;
    }

    if (address_space < 0x10) {
        memory_pointer.pointer = &sram[address_main % SRAM_SIZE];
        memory_pointer.valid = true;
        return memory_pointer;
    }

    SDL_Log("WEIRD ACCESS - TOO HIGH");
    return memory_pointer;
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

Memory::AddressableRegion::AddressableRegion(
    Word base_address, 
    Word length, 
    std::function<Byte(Byte, Byte)> write, 
    std::function<Byte(Byte)> read
) :
base_address(base_address),
length(length),
write(write),
read(read)
{} 
