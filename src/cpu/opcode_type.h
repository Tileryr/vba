#ifndef OPCODE_TYPE_INCLUDED
#define OPCODE_TYPE_INCLUDED

enum OpcodeType
{
    BRANCH,
    BX,
    SWI,
    UNDEFINED,

    
    ALU,
    MULTIPLY
};

enum class DataProcessingInstructionType 
{
    AND = 0x0,
    EOR = 0x1,
    SUB = 0x2,
    RSB = 0x3,
    ADD = 0x4,
    ADC = 0x5,
    SBC = 0x6,
    RSC = 0x7,
    TST = 0x8,
    TEQ = 0x9,
    CMP = 0xA,
    CMN = 0xB,
    ORR = 0xC,
    MOV = 0xD,
    BIC = 0xE,
    MVN = 0xF
};


#endif