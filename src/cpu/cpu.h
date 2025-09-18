#ifndef CPU_INCLUDED
#define CPU_INCLUDED

#include <string>

#include "cpu_types.h"
#include "psr.h"
#include "register.h"
#include "./opcodes/opcode_types.h"

#include "src/memory.h"

#include "../utils.h"

#define KEY_INPUT_ADDRESS 0x04000130
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

        ArmOpcodeType decode_opcode_arm(Word opcode);
        ThumbOpcodeType decode_opcode_thumb(HalfWord opcode);

        void warn(const char * msg);
        
        void arm_opcode_branch(Word opcode);
        void arm_opcode_branch_exchange(Word opcode);
        void arm_opcode_software_interrupt(Word opcode);
        void arm_opcode_undefined_intruction(Word opcode);
        void arm_opcode_data_processing(Word opcode);
        void arm_opcode_multiply(Word opcode);
        void arm_opcode_multiply_long(Word opcode);
        void arm_opcode_psr_transfer(Word opcode);
        void arm_opcode_single_data_transfer(Word opcode);
        void arm_opcode_half_word_signed_data_transfer(Word opcode);
        void arm_opcode_block_data_transfer(Word opcode);
        void arm_opcode_swap(Word opcode);

        void thumb_opcode_move_shifted_register(HalfWord opcode);
        void thumb_opcode_add_subtract(HalfWord opcode);
        void thumb_opcode_move_compare_add_subtract(HalfWord opcode);
        void thumb_opcode_alu_operations(HalfWord opcode);
        void thumb_opcode_hi_register_operations_branch_exchange(HalfWord opcode);
        void thumb_opcode_pc_relative_load(HalfWord opcode);
        void thumb_opcode_load_store_register_offset(HalfWord opcode);
        void thumb_opcode_load_store_sign_extended_byte_halfword(HalfWord opcode);
        void thumb_opcode_load_store_immediate_offset(HalfWord opcode);
        void thumb_opcode_load_store_halfword(HalfWord opcode);
        void thumb_opcode_sp_relative_load_store(HalfWord opcode);
        void thumb_opcode_load_address(HalfWord opcode);
        void thumb_opcode_add_offset_to_stack_pointer(HalfWord opcode);
        void thumb_opcode_push_pop_registers(HalfWord opcode);
        void thumb_opcode_multiple_load_store(HalfWord opcode);

        void thumb_opcode_conditional_branch(HalfWord opcode);
        void thumb_opcode_software_interrupt(HalfWord opcode);
        void thumb_opcode_unconditional_branch(HalfWord opcode);
        void thumb_opcode_long_branch_with_link(HalfWord opcode);
        
    public:
        ARM7TDMI();

        Memory memory;
        int runs = 0;
        
        Word read_word_from_memory(Word address);
        HalfWord read_halfword_from_memory(Word address);

        void write_word_to_memory(Word address, Word value);
        void write_halfword_to_memory(Word address, HalfWord value);
        
        Byte * memory_region(Word address);

        Word read_register(int register_number);
        void write_register(int register_number, Word register_value);

        bool condition_field(int condition_code);
        bool is_priviledged();
        // Exception Functions
        void run_exception(Exception exception_type);
        void start_interrupt();
        void return_from_interrupt();
        
        void run_next_opcode();

        void skip_bios();
} ARM7TDMI;

#endif