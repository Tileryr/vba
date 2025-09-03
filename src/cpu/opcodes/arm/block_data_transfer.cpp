#include "block_data_transfer.h"
#include "src/utils.h"

OpcodeBlockDataTransfer::OpcodeBlockDataTransfer() {}
OpcodeBlockDataTransfer::OpcodeBlockDataTransfer(Word opcode) {
    p = Utils::read_bit(opcode, 24);
    u = Utils::read_bit(opcode, 23);
    s = Utils::read_bit(opcode, 22);
    w = Utils::read_bit(opcode, 21);
    l = Utils::read_bit(opcode, 20);

    base_register = Utils::read_bit_range(opcode, 16, 19);
    register_list = Utils::read_bit_range(opcode, 0,  15);
}

void OpcodeBlockDataTransfer::run(ARM7TDMI * cpu) {
    Word base_address = cpu->read_register(this->base_register);

    if (cpu->is_priviledged() && this->s == 1) {
        cpu->warn("Block Data Transfer - not priviledged && s == 1");
    }
    if (this->base_register == REGISTER_PC) {
        cpu->warn("Block Data Transfer - base_register == PC");
    }

    Word offset = 0;
    Word write_back_address = base_address;
    bool pc_in_transfer_list = false;

    bool empty_register_list = this->register_list == 0b0;
    if (empty_register_list) {
        this->register_list = 0x8000;
        offset = 0x3C;
    }

    for (int i = 0; i < 16; i++) {
        bool transfer_register = Utils::read_bit(this->register_list, i);
        if (transfer_register) {
            offset += 4;
            if (i == 15) {
                pc_in_transfer_list = true;
            }
        }
    }
    write_back_address += this->u ? offset : -offset;

    Word current_address = base_address;
    if (this->u == 0) { // Down
        current_address = write_back_address;
    }

    bool pre_increment = this->u ? this->p : !this->p;

    RegisterSet * target_register_set = cpu->current_register_set();
    if ((this->s == 1) && (!pc_in_transfer_list) || (this->l == 0)) {
        target_register_set = &cpu->registers_user;
        if (this->w == 1) {
            cpu->warn("Block Data Transfer - writeback and using user register bank");
        }
    }

    if (this->w == 1) {
        cpu->write_register(this->base_register, write_back_address);
    }

    for (int i = 0; i < 16; i++) {
        bool transfer_register = Utils::read_bit(this->register_list, i);
        if (!transfer_register) {continue;}

        if (pre_increment == 1) {
            current_address += 4;
        }

        if (this->l == 0) { // Store | Register -> Memory
            bool stored_register_is_base_register = i == this->base_register;
            bool first_stored_register = Utils::read_bit_range(this->register_list, 0, i - 1) == 0;
            if (stored_register_is_base_register && first_stored_register) {
                cpu->write_word_to_memory(current_address, base_address);
            } else {
                Word register_value = *target_register_set->registers[i];
                if (i == REGISTER_PC) {
                    register_value += cpu->cpsr.t == STATE_ARM ? 12 : 6;
                }
                cpu->write_word_to_memory(current_address, register_value);
            }

        } else { // Load | Memory -> Register
            target_register_set->write_register(i, cpu->read_word_from_memory(current_address));
            if (this->s == 1 && i == REGISTER_PC) {
                cpu->cpsr = cpu->current_register_set()->spsr;
            }
        }

        if (pre_increment == 0) {
            current_address += 4;
        }
    }
}

OpcodeBlockDataTransferBuilder::OpcodeBlockDataTransferBuilder(bool load, Byte base_register) {
    product.l = load;
    product.base_register = base_register;
}

OpcodeBlockDataTransferBuilder& OpcodeBlockDataTransferBuilder::set_flags(bool pre, bool up, bool s_flag, bool writeback) {
    product.p = pre;
    product.u = up;
    product.s = s_flag;
    product.w = writeback;
    return *this;
}
OpcodeBlockDataTransferBuilder& OpcodeBlockDataTransferBuilder::set_register_list(HalfWord register_list) {
    product.register_list = register_list;
    return *this;
}

OpcodeBlockDataTransfer OpcodeBlockDataTransferBuilder::get_product() {
    return product;
}