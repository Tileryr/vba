#include <cassert>
#include <cstdlib>
#include <string.h>
#include <sys/types.h>
#include <uchar.h>
#include "../utils.h"

typedef u_int8_t  Byte;
typedef u_int16_t HalfWord;
typedef u_int32_t Word;

enum CPUState {
    STATE_ARM = 0,
    STATE_THUMB = 1
};

enum SpecialRegisters {
    REGISTER_SP = 13,
    REGISTER_LS = 14,
    REGISTER_PC = 15
};

enum OperatingMode {
    MODE_USER       = 0b10000,
    MODE_FIQ        = 0b10001,
    MODE_IRQ        = 0b10010,
    MODE_SUPERVISOR = 0b10011,
    MODE_ABORT      = 0b10111,
    MODE_SYSTEM     = 0b11111,
    MODE_UNDEFINED  = 0b11011
};

typedef struct ProgramStatusRegister {
    ProgramStatusRegister();
    Word value();
    // Flags
    bool n;
    bool z;
    bool c;
    bool v;

    bool i; // IRQ Disable;
    bool f; // FIQ Disable;
    CPUState t; // State Bit
    OperatingMode mode;
} PSR;

typedef struct RegisterSet {
    RegisterSet();

    Word * registers[16];
   
    ProgramStatusRegister spsr;
} RegisterSet;

typedef struct ARM7TDMI {
    private:
        RegisterSet registers_user;
        RegisterSet registers_fiq;
        RegisterSet registers_irq;
        RegisterSet registers_supervisor;
        RegisterSet registers_abort;
        RegisterSet registers_undefined;

        ProgramStatusRegister cpsr;

        RegisterSet mode_to_register_set(OperatingMode mode);
        RegisterSet current_register_set();

        void set_irq(bool irq_on);

        void set_fiq(bool fiq_on);

        void trigger_exception(OperatingMode new_mode, unsigned int exception_vector, int pc_offset);
    public:
        ARM7TDMI();
        
        


        Word read_register(int register_number);
        
        void write_register(int register_number, Word register_value);


        bool condition_field(int condition_code);


} cpu;

