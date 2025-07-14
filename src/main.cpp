#include "./cpu/cpu.h"
#include "./utils.h"

#include <stdlib.h>
#include <cstdio>

struct ARM7TDMI ArmCPU;

int main(int argc, char **argv )
{
    ArmCPU.memory = (u_int8_t *)malloc(32);
    unsigned int test = 0b1111;
    int extended = Utils::sign_extend(test, 4);
    printf("%d", extended);
    return 0;
}