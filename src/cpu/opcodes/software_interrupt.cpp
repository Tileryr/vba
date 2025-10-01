#include <SDL3/SDL.h>
#include "src/cpu/cpu.h"
 
void ARM7TDMI::emulate_software_interrupt(Word opcode) {
    SDL_Log("SWI, -pc: 0x%0x opcode: 0x%0x", read_register(REGISTER_PC), opcode);
    switch (opcode)
    {
        case 0x2: { // HALT
            break;
        }
        case 0x5: { // VBLANK HALT
            break;
        }
        case 0x6: { // DIVISION
            int32_t number = read_register(0);
            int32_t denom = read_register(1);

            write_register(0, number/denom);
            write_register(1, number%denom);
            write_register(3, abs(number/denom));
            break;
        }
        case 0x10: { // BITUNPACK
            SDL_TriggerBreakpoint();
            Word source_address = read_register(0);
            Word destination_address = read_register(1);

            Word unpack_info_address = read_register(2);

            Word source_data_length = read_halfword_from_memory(unpack_info_address);
            Word source_data_width = memory.read_from_memory(unpack_info_address+2);
            Word destination_data_width = memory.read_from_memory(unpack_info_address+3);
            Word data_offset = read_word_from_memory(unpack_info_address+4);

            Word next_write_address = 0;
            Word current_write_value = 0;

            for (int i = 0; i < source_data_length; i++) {
                Byte current_source_data = memory.read_from_memory(source_address + i);
                Word source_unit_counter_base = i*(8/source_data_width);

                for (int bit = 0; bit < 8; bit += source_data_width) {
                    Word source_unit_counter = source_unit_counter_base + bit/source_data_width;
                    Word current_write_address = ((source_unit_counter*destination_data_width)/8);
                    current_write_address = (current_write_address/4)*4;
  
                    Byte source_unit = Utils::read_bit_range(current_source_data, bit, (bit+source_data_width)-1);
                    source_unit += data_offset;

                    // if (source_unit != 0) {
                    //     SDL_Log("STOP");
                    // } 

                    Word segment = ((source_unit_counter % (32/destination_data_width)) * destination_data_width);
                    current_write_value |= source_unit << segment; 

                    if (current_write_address != next_write_address) {
                        SDL_Log("destination: %0x, value: %0x", next_write_address + destination_address, current_write_value);
                        write_word_to_memory(next_write_address + destination_address, current_write_value);
                        current_write_value = 0;
                        next_write_address = current_write_address;
                    }
                }
            }
            break;
        }
        default:
            SDL_TriggerBreakpoint();
            run_exception(EXCEPTION_SOFTWARE_INTERRUPT);
            break;
    }
}