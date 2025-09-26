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

    switch (address_space) {
        case 0x0:
            if (address_main < 0x4000) {
                SDL_Log("WEIRD READ - BIOS, %0x", address);
                // SDL_TriggerBreakpoint();
            } else {
                SDL_Log("WEIRD READ - UNUSED (0x0)");
            }
            return 0x0;
        case 0x1:
            SDL_Log("WEIRD READ - UNUSED");
            return 0x0;
        case 0x2:
            return wram_board[address_main % WRAM_BOARD_SIZE];
        case 0x3:
            return wram_chip[address_main % WRAM_CHIP_SIZE];
        case 0x4:
            if (address_main < IO_REGISTERS_SIZE) {
                return io_registers[address_main];
            }
            SDL_Log("WEIRD READ - IO_AREA");
            SDL_TriggerBreakpoint();
            return 0x0;
        case 0x5:
            return palette_ram[address_main % PALETTE_RAM_SIZE];
        case 0x6: {
            Word vram_address = address_main % 0x20000;
            if (vram_address < 0x10000) {
                return vram[vram_address];
            } else {
                return vram[0x10000 + (vram_address % 0x8000)];
            }
        }
        case 0x7:
            return oam[address_main % OAM_SIZE];
    }

    if (address_space < 0xE) {
        return game_pak_rom[address - (0x08 << 24)];
    }

    if (address_space < 0x10) {
        return sram[address_main % SRAM_SIZE];
    }

    SDL_Log("WEIRD READ - TOO HIGH");
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

    switch (address_space) {
        case 0x0:
            SDL_Log("WEIRD WRITE - BIOS / UNUSED");
            return;
        case 0x1:
            SDL_Log("WEIRD WRITE - UNUSED");
            return;
        case 0x2:
            wram_board[address_main % WRAM_BOARD_SIZE] = value;
            return;
        case 0x3:
            wram_chip[address_main % WRAM_CHIP_SIZE] = value;
            return;
        case 0x4:
            if (address_main < IO_REGISTERS_SIZE) {
                io_registers[address_main] = value;
                return;
            }
            SDL_Log("WEIRD WRITE - IO_AREA");
            // SDL_TriggerBreakpoint();
            return;
        case 0x5:
            palette_ram[address_main % PALETTE_RAM_SIZE] = value;
            return;
        case 0x6: {
            Word vram_address = address_main % 0x20000;
            if (vram_address < 0x10000) {
                vram[vram_address] = value;
            } else {
                vram[0x10000 + (vram_address % 0x8000)] = value;
            }
            return;
        }
        case 0x7:
            oam[address_main % OAM_SIZE] = value;
            return;
    }

    if (address_space < 0xE) {
        game_pak_rom[address - (0x08 << 24)] = value;
        return;
    }

    if (address_space < 0x10) {
        sram[address_main % SRAM_SIZE] = value;
    }

    SDL_Log("WEIRD WRITE - TOO HIGH");
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

// Byte * Memory::memory_region(Word address) {
//     return &memory[address];
// }

