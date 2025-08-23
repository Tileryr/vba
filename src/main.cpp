#include "./cpu/cpu.h"
#include "./utils.h"
#include "./cpu/alu.h"

#include <stdlib.h>
#include <cstdio>

struct ARM7TDMI ArmCPU;
CpuALU alu;

int main(int argc, char **argv )
{   
    
    ArmCPU.memory = (u_int8_t *)malloc(32);
    int8_t test = 0b10000000;
    // u_int8_t test_unsigned = test;

    // alu.add(2, 1, __UINT32_MAX__);
    bool c = 0;

    Word result = alu.subtract(2, 5, 5, -c + 1);
    
    for (int i = 0; i < 16; i++) {
        printf("%d", Utils::read_bit(0b1010101010101010, i));
    }
    
    return 0;
}