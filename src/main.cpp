#include "./cpu/cpu.h"

#include <cstdio>

struct ARM7TDMI ArmCPU;

int main(int argc, char **argv)
{   
    unsigned int *test;
    *test = 0b00011100;
    *test = Utils::read_bit_range(test, 1, 4);
    ArmCPU.write_register(14, 5);
    printf("%b", *test);
    return 0;
}