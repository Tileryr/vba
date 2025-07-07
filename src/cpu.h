#include <cstdlib>
#include <string.h>
#include <sys/types.h>
#include "./utils.h"

typedef u_int8_t  Byte;
typedef u_int16_t HalfWord;
typedef u_int32_t Word;

enum CPUState {
    STATE_ARM,
    STATE_THUMB
};

enum SpecialRegisters {
    REGISTER_SP = 13,
    REGISTER_LS = 14,
    REGISTER_PC = 15
};

enum OperatingMode {
    MODE_USER,
    MODE_FIQ,
    MODE_IRQ,
    MODE_SUPERVISOR,
    MODE_ABORT,
    MODE_SYSTEM,
    MODE_UNDEFINED
};
typedef struct ProgramStatusRegister {
    
    Word value() {
        Word * value = 0;
        Utils::write_bit((int *)value, 31, n);
        Utils::write_bit((int *)value, 30, z);
        Utils::write_bit((int *)value, 29, c);
        Utils::write_bit((int *)value, 28, v);
        Utils::write_bit((int *)value, 27, q);
        Utils::write_bit((int *)value, 7, i);
        Utils::write_bit((int *)value, 6, f);
        Utils::write_bit((int *)value, 5, t);
        Utils::write_bit_range((int *)value, 0, 4, mode_bits);
        return *value;
    };
    
    bool n;
    bool z;
    bool c;
    bool v;
    bool q; // Not sure if used in GBA.

    bool i; // IRQ Disable;
    bool f; // FIQ Disable;
    bool t; // State Bit

    HalfWord mode_bits;
} PSR;

typedef struct RegisterSet {
    RegisterSet()
    {
        for (int i = 0; i < 16; i++) {
            registers[i] = (Word *) malloc(sizeof(Word));
        }

        cpsr = (ProgramStatusRegister *) malloc(sizeof(ProgramStatusRegister));
        spsr = (ProgramStatusRegister *) malloc(sizeof(ProgramStatusRegister));
    };

    Word * registers[16];
    ProgramStatusRegister * cpsr;
    ProgramStatusRegister * spsr;
} RegisterSet;

typedef struct ARM7TDMI {
    ARM7TDMI()
    {
        RegisterSet * register_sets[5] = {&registers_fiq, &registers_irq, &registers_supervisor, &registers_abort, &registers_undefined};
        free(registers_user.spsr);

        for (int i = 0; i < 2; i++) {
            RegisterSet * current = register_sets[i];

            for (int i = 0; i < 13; i++) {
                free(current->registers[i]);
                current->registers[i] = registers_user.registers[i];
            }

            free(current->registers[REGISTER_PC]);
            current->registers[REGISTER_PC] = registers_user.registers[REGISTER_PC];

            free(current->cpsr);
            current->cpsr = registers_user.cpsr;
        }

        for (int i = 8; i < 13; i++) {
            registers_fiq.registers[i] = (Word *) malloc(sizeof(Word));
        }
    };
    
    CPUState state; 

    OperatingMode mode;

    RegisterSet registers_user;

    RegisterSet registers_fiq;
    RegisterSet registers_irq;
    RegisterSet registers_supervisor;
    RegisterSet registers_abort;
    RegisterSet registers_undefined;
} cpu;

