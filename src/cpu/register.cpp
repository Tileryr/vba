#include <stdlib.h>
#include "register.h"

RegisterSet::RegisterSet()
{
    for (int i = 0; i < 16; i++) {
        registers[i] = (Word *) malloc(sizeof(Word));
    }
};
