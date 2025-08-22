#ifndef CPU_INCLUDED
#define CPU_INCLUDED

#include "cpu_types.h"
#include "psr.h"
#include "register.h"
#include "opcode_type.h"

#include "../utils.h"

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

        OpcodeType decode_opcode_arm(Word opcode);

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
        void warn(char* msg);
    public:
        ARM7TDMI();

        Byte * memory;

        Word read_register(int register_number);
        void write_register(int register_number, Word register_value);

        bool condition_field(int condition_code);

        // Exception Functions
        void run_exception(Exception exception_type);
        void run_next_opcode();
} cpu;

#endif