#ifndef OPCODE_TYPE_INCLUDED
#define OPCODE_TYPE_INCLUDED

enum OpcodeType
{
    BRANCH,
    BX,
    SWI,
    UNDEFINED,

    
    ALU,
    MULTIPLY,
    MULTIPLY_LONG,
};

#endif