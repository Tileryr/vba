#ifndef CPU_INCLUDED
#define CPU_INCLUDED

#include "cpu_types.h"
#include "psr.h"
#include "register.h"

#include "../utils.h"

typedef struct ARM7TDMI {
    public:
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

        void trigger_exception(OperatingMode new_mode, unsigned int exception_vector, int priority, int pc_offset);
        void return_from_exception();

    public:
        ARM7TDMI();

        Word read_register(int register_number);
        
        void write_register(int register_number, Word register_value);

        bool condition_field(int condition_code);

        // Exception Functions
        void exception_reset();
        void exception_undefined_instruction();
        void exception_software_interrupt();
        void exception_prefetch_abort();
        void exception_data_abort();
        void exception_interrupt();
        void exception_fast_interrupt();
} cpu;

#endif