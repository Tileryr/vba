#ifndef PSR_INCLUDED
#define PSR_INCLUDED

#include "cpu_types.h"

enum CPUState {
    STATE_ARM = 0,
    STATE_THUMB = 1
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
    bool n; // Negative
    bool z; // Zero
    bool c; // Carry
    bool v; // Overflow

    bool i; // IRQ Disable;
    bool f; // FIQ Disable;
    CPUState t; // State Bit
    OperatingMode mode;
} PSR;

#endif