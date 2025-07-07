#include "./cpu.h"

#include <cstdio>

struct ARM7TDMI ArmCPU;

int main(int argc, char **argv)
{   
    int *test;
    *test = 0b00011100;
    *test = Utils::read_bit_range(test, 1, 4);
    *ArmCPU.registers_user.registers[REGISTER_PC] = 5;
    printf("%b", *test);
    return 0;
}