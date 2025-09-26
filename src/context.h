#ifndef CONTEXT_INCLUDED
#define CONTEXT_INCLUDED

#include "src/cpu/cpu.h"
#include "src/cpu/irq_manager.h"
#include "src/memory.h"

typedef struct Context {
    ARM7TDMI * cpu;
    Memory * memory;
} Context;

#endif