#include "cpu.h"
#include "../utils.h"

#include <cassert>
#include <cstdlib>

#include <SDL3/SDL.h>
#include "src/cpu/opcodes/arm/data_processing.h"
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
    *new_register_set->registers[REGISTER_LR] = ls_value;

    new_register_set->spsr = cpsr;
    cpsr.t = STATE_THUMB;
    cpsr.mode = new_mode;
    cpsr.i = true;

    write_register(REGISTER_PC, exception_vector);
};

Word ARM7TDMI::read_word_from_memory(Word address) {
    if (address < 0x10000000) {
        return memory.read_word_from_memory(address);
    } else {
        return read_register(REGISTER_PC) + 8;
    }
}

HalfWord ARM7TDMI::read_halfword_from_memory(Word address) {
    return memory.read_halfword_from_memory(address);
}

void ARM7TDMI::write_word_to_memory(Word address, Word value) {
    memory.write_word_to_memory(address, value);
}

void ARM7TDMI::write_halfword_to_memory(Word address, HalfWord value) {
    memory.write_halfword_to_memory(address, value);
}

// Public

ARM7TDMI::ARM7TDMI() : memory(0xFFFFFFF) {
    registers_user = RegisterSet();

    RegisterSet * register_sets[5] = {&registers_fiq, &registers_irq, &registers_supervisor, &registers_abort, &registers_undefined};

    for (int i = 0; i < 5; i++) {
        RegisterSet * current = register_sets[i];
        *current = RegisterSet(); 

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
            condition = cpsr.z == 1; break;
        case 0x1:
            condition = cpsr.z == 0; break;
        case 0x2:
            condition = cpsr.c == 1; break;
        case 0x3:
            condition = cpsr.c == 0; break;
        case 0x4:
            condition = cpsr.n == 1; break;
        case 0x5:
            condition = cpsr.n == 0; break;
        case 0x6:
            condition = cpsr.v == 1; break;
        case 0x7:
            condition = cpsr.v == 0; break;
        case 0x8:
            condition = cpsr.c == 1 && cpsr.z == 0; break;
        case 0x9:
            condition = cpsr.c == 0 || cpsr.z == 1; break;
        case 0xA:
            condition = cpsr.n == cpsr.v; break;
        case 0xB:
            condition = cpsr.n != cpsr.v; break;
        case 0xC:
            condition = cpsr.z == 0 && cpsr.n == cpsr.v; break;
        case 0xD:
            condition = cpsr.z == 1 || cpsr.n != cpsr.v; break;
        case 0xE:
            condition = true; break;
        case 0xF:
            condition = false; break;
    }

    return condition;
}

bool ARM7TDMI::is_priviledged() {
    return cpsr.mode != MODE_USER;
}

Byte * ARM7TDMI::memory_region(Word address) {
    return memory.memory_region(address);
}

void ARM7TDMI::skip_bios() {
    for (Word i = 0x3007E00; i < 0x3008000; i++) {
        memory.memory[i] = 0x0;
    }
    
    for (Word i = 0; i < 13; i++) {
        *registers_user.registers[i] = 0;
    }
    *registers_supervisor.registers[REGISTER_LR] = 0;
    registers_supervisor.spsr.write_value(0); 

    *registers_irq.registers[REGISTER_LR] = 0;
    registers_irq.spsr.write_value(0); 

    *registers_supervisor.registers[REGISTER_SP] = 0x3007FE0;
    *registers_irq.registers[REGISTER_SP] = 0x03007FA0;
    *registers_user.registers[REGISTER_SP] = 0x03007F00;

    cpsr.mode = MODE_SYSTEM;
    cpsr.t = STATE_ARM;
    write_register(REGISTER_PC, GAMEPAK_ROM_START);
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

void ARM7TDMI::warn(const char * msg)
{
    return;
}

void ARM7TDMI::run_next_opcode()
{   
    Word pc = read_register(REGISTER_PC);

    if (pc >= 0x800153c) {
        // SDL_Log("BREAK");
    }

    if (pc == 0x8001d4c) {
        if (read_register(12) == 0) {
            SDL_Log("TESTS PASSED");
        } else {
            SDL_Log("FAILED: TEST %d", read_register(12));
            SDL_TriggerBreakpoint();
        }
    }

    if (cpsr.t == STATE_ARM) {
        Word opcode = read_word_from_memory(pc);
        ArmOpcodeType opcode_type = decode_opcode_arm(opcode);

        SDL_Log("pc: %08x, opcode: %08x, type: %s, register: %08x \n", pc, opcode, dissassemble_opcode(opcode).c_str(), read_register(0));
        
        Byte condition_code = Utils::read_bit_range(opcode, 28, 31);
        if (condition_field(condition_code) == false) {
            write_register(REGISTER_PC, pc + 4);
            
            return;
        }

        switch (opcode_type)
        {
            case BRANCH: arm_opcode_branch(opcode); break;
            case BX: arm_opcode_branch_exchange(opcode); break;
            case SWI: arm_opcode_software_interrupt(opcode); break;
            case UNDEFINED: arm_opcode_undefined_intruction(opcode); break;
            case ALU: arm_opcode_data_processing(opcode); break;
            case MULTIPLY: arm_opcode_multiply(opcode); break;
            case MULTIPLY_LONG: arm_opcode_multiply_long(opcode); break;
            case PSR_TRANSFER: arm_opcode_psr_transfer(opcode); break;
            case SINGLE_DATA_TRANSFER: arm_opcode_single_data_transfer(opcode); break;
            case HALF_WORD_SIGNED_DATA_TRANSFER: arm_opcode_half_word_signed_data_transfer(opcode); break;
            case BLOCK_DATA_TRANSFER: arm_opcode_block_data_transfer(opcode); break;
            case SWAP: arm_opcode_swap(opcode); break;
            default:
                break;
        }

        bool pc_changed = pc != read_register(REGISTER_PC);
        if (!pc_changed) {
            write_register(REGISTER_PC, read_register(REGISTER_PC) + 4);
        }
    } else {
        HalfWord opcode = read_halfword_from_memory(pc);
        ThumbOpcodeType opcode_type = decode_opcode_thumb(opcode);

        SDL_Log("pc: %08x, opcode: %04x, THUMB", pc, opcode);
        bool increment_pc = true;

        switch (opcode_type)
        {
            case MOVE_COMPARE_ADD_SUBTRACT_IMMEDIATE: thumb_opcode_move_compare_add_subtract(opcode); break;
            case HI_REGISTER_OPERATIONS_BRANCH_EXCHANGE: thumb_opcode_hi_register_operations_branch_exchange(opcode, &increment_pc); break;
            case LOAD_ADDRESS: thumb_opcode_load_address(opcode); break;
        }

        if (increment_pc) {
            write_register(REGISTER_PC, read_register(REGISTER_PC) + 2);
        }
    }
}

