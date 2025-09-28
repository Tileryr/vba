#ifndef MEMORY_INCLUDED
#define MEMORY_INCLUDED

#include <functional>
#include <vector>

#include "src/cpu/cpu_types.h"

#define BIOS_SIZE 0x00004000
#define WRAM_BOARD_SIZE 0x00040000
#define WRAM_CHIP_SIZE 0x00008000
#define IO_REGISTERS_SIZE 0x3FF

#define PALETTE_RAM_SIZE 0x400
#define VRAM_SIZE 0x18000
#define OAM_SIZE 0x400

#define GAME_PAK_ROM_SIZE 0x06000000
#define SRAM_SIZE 0x00010000

typedef struct Memory {
    Byte wram_board[WRAM_BOARD_SIZE];
    Byte wram_chip[WRAM_CHIP_SIZE];
    Byte io_registers[IO_REGISTERS_SIZE];

    Byte palette_ram[PALETTE_RAM_SIZE];
    Byte vram[VRAM_SIZE];
    Byte oam[OAM_SIZE];
    
    Byte game_pak_rom[GAME_PAK_ROM_SIZE];
    Byte sram[SRAM_SIZE];

    typedef struct AddressableRegion {
        AddressableRegion(Word base_address, Word length, std::function<Byte(Byte, Byte)> write, std::function<Byte(Byte)> read);
        Word base_address;
        Word length;

        std::function<Byte(Byte, Byte)> write;
        std::function<Byte(Byte)> read;
    } AddressableRegion;

    std::vector<AddressableRegion> addressable_regions;
    
    Word read_word_from_memory(Word address);
    HalfWord read_halfword_from_memory(Word address);
    Byte read_from_memory(Word address);

    void write_word_to_memory(Word address, Word value);
    void write_halfword_to_memory(Word address, HalfWord value);
    void write_to_memory(Word address, Byte value);

    typedef struct MemoryPointer {
        Byte * pointer;
        bool valid;
    } MemoryPointer;

    MemoryPointer address_to_memory_pointer(Word address);

    static Word read_word_from_memory(Byte * memory, Word address);
    static HalfWord read_halfword_from_memory(Byte * memory, Word address);
    static Byte read_from_memory(Byte * memory, Word address);

    static void write_word_to_memory(Byte * memory, Word address, Word value);
    static void write_halfword_to_memory(Byte * memory, Word address, HalfWord value);
    static void write_to_memory(Byte * memory, Word address, Byte value);
} Memory;

#endif