#include "./cpu/cpu.h"
#include "./utils.h"

#include <stdlib.h>
#include <cstdio>

struct ARM7TDMI ArmCPU;

int main(int argc, char **argv )
{
    ArmCPU.memory = (u_int8_t *)malloc(32);
    int8_t test = 0b10000000;
    // u_int8_t test_unsigned = test;

    printf("%b", Utils::rotate_right(test, 1, 8));
    return 0;
}