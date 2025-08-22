#include "cpu.h"
#include "opcode_type.h"
#include "../utils.h"

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

void ARM7TDMI::trigger_exception(OperatingMode new_mode, unsigned int exception_vector, unsigned int saved_pc_offset, int priority)
{
    RegisterSet * new_register_set = mode_to_register_set(new_mode);
    Word pc_value = read_register(REGISTER_PC);
    Word ls_value = pc_value;

    ls_value += saved_pc_offset;
    *new_register_set->registers[REGISTER_LS] = ls_value;

    new_register_set->spsr = cpsr;
    cpsr.t = STATE_ARM;
    cpsr.mode = new_mode;
    cpsr.i = true;

    write_register(REGISTER_PC, exception_vector);
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
        return *registers_user.registers[REGISTER_PC] + 0;
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

void ARM7TDMI::run_exception(Exception exception_type) {
    switch (exception_type)
    {
        case EXCEPTION_RESET:
            trigger_exception(MODE_SUPERVISOR, 0x00, 0, 1);
            cpsr.f = true;
            break;
        case EXCEPTION_UNDEFINED:
            trigger_exception(MODE_UNDEFINED, 0x04, 4, 7);
            break;
        case EXCEPTION_SOFTWARE_INTERRUPT:
            trigger_exception(MODE_SUPERVISOR, 0x08, 4, 6);
            break;
        case EXCEPTION_PREFETCH_ABORT:
            trigger_exception(MODE_ABORT, 0x0C, 4, 5);
            break;
        case EXCEPTION_DATA_ABORT:
            trigger_exception(MODE_ABORT, 0x10, 8, 2);
            break;
        case EXCEPTION_INTERRUPT:
            if (cpsr.i) break;
            trigger_exception(MODE_IRQ, 0x18, 4, 4);
            break;
        case EXCEPTION_FAST_INTERRUPT:
            if (cpsr.f) break;
            trigger_exception(MODE_FIQ, 0x1C, 4, 3);
            cpsr.f = true;
            break;
    }
};

void ARM7TDMI::warn(char* msg)
{
    return;
}

void ARM7TDMI::run_next_opcode()
{   
    Word pc = read_register(REGISTER_PC);
    Byte * current_memory_place = &memory[pc];

    if (cpsr.t == STATE_ARM) {
        Word opcode = Utils::current_word_at_memory(current_memory_place, endian_type);
        OpcodeType opcode_type = decode_opcode_arm(opcode);

        Byte condition_code = Utils::read_bit_range(opcode, 28, 31);
        if (condition_field(condition_code) == false) 
        {
            return;
        }

        switch (opcode)
        {
            case BRANCH: opcode_branch(opcode); break;
            case BX: opcode_branch_exchange(opcode); break;
            case SWI: opcode_software_interrupt(opcode); break;
            case UNDEFINED: opcode_undefined_intruction(opcode); break;
            case ALU: opcode_data_processing(opcode); break;
            case MULTIPLY: opcode_multiply(opcode); break;
            case MULTIPLY_LONG: opcode_multiply_long(opcode); break;
            case PSR_TRANSFER: opcode_psr_transfer(opcode); break;
            case SINGLE_DATA_TRANSFER: opcode_single_data_transfer(opcode); break;
            default:
                break;
        }
    }   
}

