#include "multiply.h"
#include "src/utils.h"
OpcodeMultiply::OpcodeMultiply() {}
OpcodeMultiply::OpcodeMultiply(Word opcode) {
    accumulate = Utils::read_bit(opcode, 21);
    set_condition_codes = Utils::read_bit(opcode, 20);
    register_destination = Utils::read_bit_range(opcode, 16, 19);
    rn = Utils::read_bit_range(opcode, 12, 15);
    rs = Utils::read_bit_range(opcode, 8, 11);
    rm = Utils::read_bit_range(opcode, 0, 3);
}

void OpcodeMultiply::run(ARM7TDMI * cpu) {
    Word destination_value;

    Word rn_value = cpu->read_register(rn);
    Word rs_value = cpu->read_register(rs);
    Word rm_value = cpu->read_register(rm);

    if (
        register_destination == REGISTER_PC ||
        rn == REGISTER_PC ||
        rs == REGISTER_PC ||
        rm == REGISTER_PC 
    ) {
        cpu->warn("Multiply - Invalid register value (PC)");
    }

    if ( register_destination == rm ) {
        cpu->warn("Multiply - Invalid register value (rd == rm)");
    }

    if (accumulate) {
        destination_value = (rm_value * rs_value) + rn_value;
    } else { // Multiply Only
        destination_value = rm_value * rs_value;
    }

    if (set_condition_codes) {
        cpu->cpsr.n = Utils::read_bit(31, destination_value);
        cpu->cpsr.z = destination_value == 0;
        cpu->cpsr.c = rand() & 1;
        cpu->cpsr.v; // Unaffected
    }

    cpu->write_register(register_destination, destination_value);
}

OpcodeMultiplyBuilder::OpcodeMultiplyBuilder(Byte destination_register, Byte rs, Byte rm, bool set_condition_codes) {
    product.accumulate = false;
    product.rn = 0;
    product.set_condition_codes = set_condition_codes;
    product.register_destination = destination_register;
    product.rs = rs;
    product.rm = rm;
}

OpcodeMultiplyBuilder& OpcodeMultiplyBuilder::set_accumulate(Byte accumulate_register) {
    product.accumulate = true;
    product.rn = accumulate_register;
}

OpcodeMultiply OpcodeMultiplyBuilder::get_product() {
    return product;
}