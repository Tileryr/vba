#include <stdlib.h>
#include "register.h"

RegisterSet::RegisterSet()
{
    for (int i = 0; i < 16; i++) {
        registers[i] = (Word *) malloc(sizeof(Word));
    }
};

void RegisterSet::write_register(int register_number, Word register_value) {
    if (register_number == REGISTER_PC) {
        register_value = register_value & (~1);
    }
    *(registers[register_number]) = register_value;
}