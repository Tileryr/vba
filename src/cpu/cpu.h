#ifndef CPU_INCLUDED
#define CPU_INCLUDED

#include <string>

#include "cpu_types.h"
#include "psr.h"
#include "register.h"
#include "./opcodes/opcode_types.h"

#include "../utils.h"

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 160

#define VRAM_START 0x06000000

#define BG_PALETTE_RAM_START 0x05000000
#define OBJ_PALETTE_RAM_START 0x05000200

#define GAMEPAK_ROM_START 0x08000000

enum Exception {
    EXCEPTION_RESET,
    EXCEPTION_UNDEFINED,
    EXCEPTION_SOFTWARE_INTERRUPT,
    EXCEPTION_PREFETCH_ABORT,
    EXCEPTION_DATA_ABORT,
    EXCEPTION_INTERRUPT,
    EXCEPTION_FAST_INTERRUPT
};

typedef struct ARM7TDMI {
    public:
        const Endian endian_type = ENDIAN_LITTLE;
        
        RegisterSet registers_user;
        RegisterSet registers_fiq;
        RegisterSet registers_irq;
        RegisterSet registers_supervisor;
        RegisterSet registers_abort;
        RegisterSet registers_undefined;

        ProgramStatusRegister cpsr;

        RegisterSet * mode_to_register_set(OperatingMode mode);
        RegisterSet * current_register_set();

        void set_irq(bool irq_on);

        void set_fiq(bool fiq_on);

        void trigger_exception(OperatingMode new_mode, unsigned int exception_vector, unsigned int saved_pc_offset, int priority);

        std::string dissassemble_opcode(Word opcode);
        ArmOpcodeType decode_opcode_arm(Word opcode);
        ThumbOpcodeType decode_opcode_thumb(HalfWord opcode);

        void warn(const char * msg);

        void write_word_to_memory(Word address, Word value);
        
        void opcode_branch(Word opcode);
        void opcode_branch_exchange(Word opcode);
        void opcode_software_interrupt(Word opcode);
        void opcode_undefined_intruction(Word opcode);
        void opcode_data_processing(Word opcode);
        void opcode_multiply(Word opcode);
        void opcode_multiply_long(Word opcode);
        void opcode_psr_transfer(Word opcode);
        void opcode_single_data_transfer(Word opcode);
        void opcode_half_word_signed_data_transfer(Word opcode);
        void opcode_block_data_transfer(Word opcode);
        void opcode_swap(Word opcode);
    public:
        ARM7TDMI();

        Byte * memory;
        int runs = 0;
        
        Word read_word_from_memory(Word address);
        HalfWord read_halfword_from_memory(Word address);

        Word read_register(int register_number);
        void write_register(int register_number, Word register_value);

        bool condition_field(int condition_code);
        bool is_priviledged();
        // Exception Functions
        void run_exception(Exception exception_type);
        void run_next_opcode();

        Byte * memory_region(Word address);
        void skip_bios();
} ARM7TDMI;

#endif