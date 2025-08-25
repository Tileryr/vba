#ifndef DATA_TRANSFER_INCLUDED
#define DATA_TRANSFER_INCLUDED

#include "src/cpu/cpu_types.h"
#include "src/cpu/cpu.h"

typedef struct DataTransfer
{
    DataTransfer(Word opcode);
    
    unsigned int p : 1; // Pre/Post : 24
    unsigned int u : 1; // Up/Down : 23
    
    unsigned int w : 1; // Write-back : 21
    unsigned int l : 1; // Load/Store : 20

    unsigned int base_register : 4; // 19-16
    unsigned int source_destination_register : 4; // 15-12

    unsigned int offset_register : 4; // 3 - 0

    Word calculate_address(ARM7TDMI * cpu, Word offset);

} DataTransfer;


#endif