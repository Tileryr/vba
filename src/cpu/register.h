#ifndef REGISTER_INCLUDED
#define REGISTER_INCLUDED

#include "cpu_types.h"
#include "psr.h"

enum SpecialRegisters {
    REGISTER_SP = 13,
    REGISTER_LR = 14,
    REGISTER_PC = 15
};

typedef struct RegisterSet {
    RegisterSet();
    void write_register(int register_number, Word register_value);
    Word * registers[16];
   
    ProgramStatusRegister spsr;
} RegisterSet;

#endif