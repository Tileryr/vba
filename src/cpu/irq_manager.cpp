#include <SDL3/SDL.h>
#include "src/cpu/irq_manager.h"

IrqManager::IrqManager(Memory * memory) : 
interrupt_enables(&memory->io_registers[0x200], 0, 15),
interrupt_info(&memory->io_registers[0x202], 0, 15),
interrupt_master_enable(&memory->io_registers[0x208], 0) 
{
    memory->addressable_regions.push_back(Memory::AddressableRegion(
        0x04000202, 1, 
        [](Word current_value, Word writed_value){
            return current_value = current_value & (~writed_value);
        },
        [](Word current_value){
            return current_value;
        }
    ));
}

void IrqManager::start_interrupt(Interrupt interrupt) {
    Word interrupt_enabled = interrupt_enables.get() & (1 << interrupt);
    if (interrupt_enabled == 0 || interrupt_master_enable.get() == 0) {return;}
    // if (true) {return;}
    interrupt_info.set(interrupt_info.get() | (1 << interrupt));
}