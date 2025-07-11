#include "./cpu/cpu.h"

#include <cstdio>

struct ARM7TDMI ArmCPU;

int main(int argc, char **argv )
{
    ArmCPU.write_register(7, 24);
    ArmCPU.exception_fast_interrupt();
    printf("%d", ArmCPU.read_register(7));

    return 0;
}