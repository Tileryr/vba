#include <string>
#include "src/cpu/opcodes/opcode_types.h"

std::string dissassemble_opcode_arm(ArmOpcodeType opcode_type);
std::string dissassemble_opcode_thumb(ThumbOpcodeType opcode_type);