#include "./cpu.h"
#include <cassert>

ProgramStatusRegister::ProgramStatusRegister() : mode(MODE_USER), t(STATE_ARM), f(false), i(false)
{
};

Word ProgramStatusRegister::value() {
    Word * value = 0;
    Utils::write_bit(value, 31, n);
    Utils::write_bit(value, 30, z);
    Utils::write_bit(value, 29, c);
    Utils::write_bit(value, 28, v);
    Utils::write_bit(value, 7, i);
    Utils::write_bit(value, 6, f);
    Utils::write_bit(value, 5, t);
    Utils::write_bit_range(value, 0, 4, mode);
    return *value;
};


RegisterSet::RegisterSet()
{
    for (int i = 0; i < 16; i++) {
        registers[i] = (Word *) malloc(sizeof(Word));
    }

    spsr = (ProgramStatusRegister *) malloc(sizeof(ProgramStatusRegister));
};

// Private

RegisterSet ARM7TDMI::current_register_set() 
{
    switch (cpsr.mode) {
        case MODE_USER :
            return registers_user;
        case MODE_SYSTEM:
            return registers_user;
        case MODE_FIQ:
            return registers_fiq;
        case MODE_IRQ :
            return registers_irq;
        case MODE_SUPERVISOR:
            return registers_supervisor;
        case MODE_ABORT:
            return registers_abort;
        case MODE_UNDEFINED:
            return registers_undefined;
    }
    assert(false);
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

// Public

ARM7TDMI::ARM7TDMI()
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

    return *current_register_set().registers[register_number];
};

void ARM7TDMI::write_register(int register_number, Word register_value)
{
    assert(register_number < 16);
    assert(register_number != REGISTER_PC);
    *current_register_set().registers[register_number] = register_value;
}

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