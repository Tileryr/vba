#ifndef OPCODE_DATA_PROCESSING_INCLUDED
#define OPCODE_DATA_PROCESSING_INCLUDED

#include "../../alu.h"
#include "../../cpu_types.h"
#include "../../psr.h"

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

typedef struct OpcodeDataProcess {
     enum BitShiftType {
        LSL = 0,
        LSR = 1,
        ASR = 2,
        ROR = 3
    };

    enum InstructionType 
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

    enum OperationClass {
        ARITHMETIC,
        LOGICAL
    };

    unsigned int use_immediate_operand_2 : 1;
    unsigned int instruction_type : 4;
    unsigned int set_condition_codes : 1;
    unsigned int rn : 4;
    unsigned int rd : 4;

    // If (register_op_2)
    unsigned int shift_type : 2;
    unsigned int operand_2_register : 4;
    unsigned int shift_by_register : 1;

        // If (shift_by_register)
        unsigned int shift_register : 4;
        // Else (shift_by_immediate)
        unsigned int shift_immediate_amount : 5;

    // Else (immediate_op_2)
    unsigned int immediate_ror_shift : 4;
    unsigned int operand_2_immediate : 8;

    CpuALU alu;

    u_int64_t last_result;

    bool write_result;
    BitShiftType bit_shift_type;
    OperationClass operation_class;

    OpcodeDataProcess(Word opcode);

    Word calculate_immediate_op2( Byte immediate, unsigned int ror_shift);
    u_int64_t calculate_instruction(InstructionType instruction, Word rn, Word op2, bool c_flag);
    static Word shift_op2(CpuALU alu, Word op2, Byte shift_amount, BitShiftType bit_shift_type, bool c_flag);
    void set_psr_flags(PSR * psr, u_int64_t result, OperationClass operation_class);

    Byte get_op_2_register_shift_amount(bool shift_by_register, Word shift_register_value);
    unsigned int calculate_pc_prefetch_offset();
} DataProcess;

#endif