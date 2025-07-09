#include <cassert>
#include <cstdlib>
#include <string.h>
#include <sys/types.h>
#include "../utils.h"

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
    MODE_USER       = 0b10000,
    MODE_FIQ        = 0b10001,
    MODE_IRQ        = 0b10010,
    MODE_SUPERVISOR = 0b10011,
    MODE_ABORT      = 0b10111,
    MODE_SYSTEM     = 0b11111,
    MODE_UNDEFINED  = 0b11011
};

typedef struct ProgramStatusRegister {
    
    Word value() {
        Word * value = 0;
        Utils::write_bit(value, 31, n);
        Utils::write_bit(value, 30, z);
        Utils::write_bit(value, 29, c);
        Utils::write_bit(value, 28, v);
        Utils::write_bit(value, 7, i);
        Utils::write_bit(value, 6, f);
        Utils::write_bit(value, 5, t);
        Utils::write_bit_range(value, 0, 4, mode_bits);
        return *value;
    };
    
    bool n;
    bool z;
    bool c;
    bool v;

    bool i; // IRQ Disable;
    bool f; // FIQ Disable;
    bool t; // State Bit

    OperatingMode mode_bits;
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
    private:
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
        
        RegisterSet current_register_set() 
        {
            switch (mode) {
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
        }
    public:
        Word read_register(int register_number) 
        {
            assert(register_number < 16);

            if (register_number == REGISTER_PC)
            {
                // TODO: Once memory is added chnage offset to emulate pipeling.
                return *registers_user.registers[REGISTER_PC] + 8;
            }

            return *current_register_set().registers[register_number];
        };

        void write_register(int register_number, Word register_value)
        {
            assert(register_number < 16);
            assert(register_number != REGISTER_PC);
            *current_register_set().registers[register_number] = register_value;
        }

        bool condition_field(int condition_code) 
        {
            assert(condition_code < 0x10);
            bool condition = false;
            PSR * cpsr = registers_user.cpsr;
            switch (condition_code) 
            {
                case 0x0:
                    condition = cpsr->z == 1;
                case 0x1:
                    condition = cpsr->z == 0;
                case 0x2:
                    condition = cpsr->c == 1;
                case 0x3:
                    condition = cpsr->c == 0;
                case 0x4:
                    condition = cpsr->n == 1;
                case 0x5:
                    condition = cpsr->n == 0;
                case 0x6:
                    condition = cpsr->v == 1;
                case 0x7:
                    condition = cpsr->v == 0;
                case 0x8:
                    condition = cpsr->c == 1 && cpsr->z == 0;
                case 0x9:
                    condition = cpsr->c == 0 || cpsr->z == 1;
                case 0xA:
                    condition = cpsr->n == cpsr->v;
                case 0xB:
                    condition = cpsr->n != cpsr->v;
                case 0xC:
                    condition = cpsr->z == 0 && cpsr->n == cpsr->v;
                case 0xD:
                    condition = cpsr->z == 1 || cpsr->n != cpsr->v;
                case 0xE:
                    condition = true;
                case 0xF:
                    condition = false;
            }

            return condition;
        }
} cpu;

