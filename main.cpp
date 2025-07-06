#include "cpu.h"
#include <cstdio>

struct ARM7TDMI ArmCPU;

int main(int argc, char **argv)
{   
    *ArmCPU.registers_user.registers[PC] = 5;
    printf("FIQ%d", *ArmCPU.registers_fiq.registers[PC]);
    return 0;
}