#ifndef REGISTER_INCLUDED
#define REGISTER_INCLUDED

#include "cpu_types.h"
#include "psr.h"

enum SpecialRegisters {
    REGISTER_SP = 13,
    REGISTER_LS = 14,
    REGISTER_PC = 15
};

typedef struct RegisterSet {
    RegisterSet();

    Word * registers[16];
   
    ProgramStatusRegister spsr;
} RegisterSet;

#endif