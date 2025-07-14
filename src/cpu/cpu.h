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

enum ExceptionReturnType {
    E_RETURN_NONE,
    E_RETURN_NEXT,
    E_RETURN_RETRY,
    E_RETURN_DATA_ABORT
};

typedef struct ARM7TDMI {
    public:
        RegisterSet registers_user;
        RegisterSet registers_fiq;
        RegisterSet registers_irq;
        RegisterSet registers_supervisor;
        RegisterSet registers_abort;
        RegisterSet registers_undefined;

        ProgramStatusRegister cpsr;
        ExceptionReturnType current_exception_return_type;

        RegisterSet * mode_to_register_set(OperatingMode mode);
        RegisterSet * current_register_set();

        void set_irq(bool irq_on);

        void set_fiq(bool fiq_on);

        void trigger_exception(OperatingMode new_mode, unsigned int exception_vector, int priority, ExceptionReturnType return_mode);

        OpcodeType decode_opcode_arm(Word opcode);

        void opcode_branch(Word opcode);
        void opcode_branch_exchange(Word opcode);
        void opcode_software_interrupt(Word opcode);
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