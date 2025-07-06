#include <cstdlib>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

typedef u_int8_t  Byte;
typedef u_int16_t HalfWord;
typedef u_int32_t Word;

const bool ARM_STATE   = false;
const bool THUMB_STATE = true;

const int SP = 13;
const int LS = 14;
const int PC = 15;

enum OperatingMode {
    USER,
    FIQ,
    IRQ,
    SUPERVISOR,
    ABORT,
    SYSTEM,
    UNDEFINED
};


typedef struct RegisterSet {
    RegisterSet()
    {
        for (int i = 0; i < 16; i++) {
            registers[i] = (Word *) malloc(sizeof(Word));
        }

        spsr = (Word *) malloc(sizeof(Word));
    };

    Word * registers[16];
    Word * cpsr;
    Word * spsr;
} RegisterSet;

typedef struct ARM7TDMI {
    ARM7TDMI()
    {
        RegisterSet * register_sets[5] = {&registers_fiq, &registers_irq, &registers_supervisor, &registers_abort, &registers_undefined};
        for (int i = 0; i < 2; i++) {
            RegisterSet * current = register_sets[i];

            for (int i = 0; i < 13; i++) {
                current->registers[i] = registers_user.registers[i];
            }

            current->registers[PC] = registers_user.registers[PC];
            current->cpsr = registers_user.cpsr;
        }

        for (int i = 8; i < 13; i++) {
            registers_fiq.registers[i] = (Word *) malloc(sizeof(Word));
        }
    };
    
    bool state; 

    OperatingMode mode;

    RegisterSet registers_user;

    RegisterSet registers_fiq;
    RegisterSet registers_irq;
    RegisterSet registers_supervisor;
    RegisterSet registers_abort;
    RegisterSet registers_undefined;
} cpu;

