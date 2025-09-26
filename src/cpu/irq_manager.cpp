#include "src/cpu/irq_manager.h"

IrqManager::IrqManager(Memory * memory) : 
interrupt_info(&memory->io_registers[0x202], 0, 15) {

}

void IrqManager::start_interrupt(Interrupt interrupt) {
    interrupt_info.set(interrupt_info.get() | (1 << interrupt));
}