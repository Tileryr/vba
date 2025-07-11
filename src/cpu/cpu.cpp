#include "cpu.h"

#include <cassert>
#include <cstdlib>

// Private

RegisterSet * ARM7TDMI::mode_to_register_set(OperatingMode mode) 
{
    switch (mode) {
        case MODE_USER :
            return &registers_user;
        case MODE_SYSTEM:
            return &registers_user;
        case MODE_FIQ:
            return &registers_fiq;
        case MODE_IRQ :
            return &registers_irq;
        case MODE_SUPERVISOR:
            return &registers_supervisor;
        case MODE_ABORT:
            return &registers_abort;
        case MODE_UNDEFINED:
            return &registers_undefined;
    }
    assert(false);
}
RegisterSet * ARM7TDMI::current_register_set() 
{
    return mode_to_register_set(cpsr.mode);
}

void ARM7TDMI::set_irq(bool irq_on)
{
    bool is_privileged = cpsr.mode != MODE_USER;
    if (!is_privileged) return;

    cpsr.i = irq_on ? 0 : 1;
}

void ARM7TDMI::set_fiq(bool fiq_on)
{
    bool is_privileged = cpsr.mode != MODE_USER;
    if (!is_privileged) return;

    cpsr.f = fiq_on ? 0 : 1;
}

void ARM7TDMI::trigger_exception(OperatingMode new_mode, unsigned int exception_vector, int priority, int pc_offset)
{
    RegisterSet * new_register_set = mode_to_register_set(new_mode);

    *new_register_set->registers[REGISTER_LS] = read_register(REGISTER_PC) + pc_offset;

    new_register_set->spsr = cpsr;
    cpsr.t = STATE_ARM;
    cpsr.mode = new_mode;
    cpsr.i = true;

    write_register(REGISTER_PC, exception_vector);
};

void ARM7TDMI::return_from_exception() 
{
    cpsr = current_register_set()->spsr;
};

// Public

ARM7TDMI::ARM7TDMI()
{
    RegisterSet * register_sets[5] = {&registers_fiq, &registers_irq, &registers_supervisor, &registers_abort, &registers_undefined};

    for (int i = 0; i < 2; i++) {
        RegisterSet * current = register_sets[i];

        for (int i = 0; i < 13; i++) {
            free(current->registers[i]);
            current->registers[i] = registers_user.registers[i];
        }

        free(current->registers[REGISTER_PC]);
        current->registers[REGISTER_PC] = registers_user.registers[REGISTER_PC];
    }

    for (int i = 8; i < 13; i++) {
        registers_fiq.registers[i] = (Word *) malloc(sizeof(Word));
    }
};

Word ARM7TDMI::read_register(int register_number)
{
    assert(register_number < 16);

    if (register_number == REGISTER_PC)
    {
        // TODO: Once memory is added chnage offset to emulate pipeling.
        return *registers_user.registers[REGISTER_PC] + 8;
    }

    return *(current_register_set()->registers[register_number]);
};

void ARM7TDMI::write_register(int register_number, Word register_value)
{
    assert(register_number < 16);
    *(current_register_set()->registers[register_number]) = register_value;
};

bool ARM7TDMI::condition_field(int condition_code) 
{
    assert(condition_code < 0x10);
    bool condition = false;
    switch (condition_code) 
    {
        case 0x0:
            condition = cpsr.z == 1;
        case 0x1:
            condition = cpsr.z == 0;
        case 0x2:
            condition = cpsr.c == 1;
        case 0x3:
            condition = cpsr.c == 0;
        case 0x4:
            condition = cpsr.n == 1;
        case 0x5:
            condition = cpsr.n == 0;
        case 0x6:
            condition = cpsr.v == 1;
        case 0x7:
            condition = cpsr.v == 0;
        case 0x8:
            condition = cpsr.c == 1 && cpsr.z == 0;
        case 0x9:
            condition = cpsr.c == 0 || cpsr.z == 1;
        case 0xA:
            condition = cpsr.n == cpsr.v;
        case 0xB:
            condition = cpsr.n != cpsr.v;
        case 0xC:
            condition = cpsr.z == 0 && cpsr.n == cpsr.v;
        case 0xD:
            condition = cpsr.z == 1 || cpsr.n != cpsr.v;
        case 0xE:
            condition = true;
        case 0xF:
            condition = false;
    }

    return condition;
}

void ARM7TDMI::exception_reset() {
    trigger_exception(MODE_SUPERVISOR, 0x00, 1, 0);
    cpsr.f = true;
}
void ARM7TDMI::exception_undefined_instruction() {
    trigger_exception(MODE_UNDEFINED, 0x04, 7, 4);
}
void ARM7TDMI::exception_software_interrupt() {
    trigger_exception(MODE_SUPERVISOR, 0x08, 6, 4);
}
void ARM7TDMI::exception_prefetch_abort() {
    trigger_exception(MODE_ABORT, 0x0C, 5, 8);
}
void ARM7TDMI::exception_data_abort() {
    trigger_exception(MODE_ABORT, 0x10, 2, 12);
}
void ARM7TDMI::exception_interrupt() {
    trigger_exception(MODE_IRQ, 0x18, 4, 8);
}
void ARM7TDMI::exception_fast_interrupt() {
    trigger_exception(MODE_FIQ, 0x1C, 3, 8);
    cpsr.f = true;
}
