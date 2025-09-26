#ifndef IRQ_MANAGER_INCLUDED
#define IRQ_MANAGER_INCLUDED

#include "src/cpu/bit_region.h"

enum Interrupt {
    INTERRUPT_VBLANK,
    INTERRUPT_HBLANK,
    INTERRUPT_VCOUNT,
    INTERRUPT_TIMER_0,
    INTERRUPT_TIMER_1,
    INTERRUPT_TIMER_2,
    INTERRUPT_TIMER_3,
    INTERRUPT_SERIAL_COMMUNICATION,
    INTERRUPT_DMA_0,
    INTERRUPT_DMA_1,
    INTERRUPT_DMA_2,
    INTERRUPT_DMA_3,
    INTERRUPT_KEYPAD,
    INTERRUPT_CARTRIDGE,
};

typedef struct IrqManager {
    IrqManager(Memory * memory);

    BitRegion interrupt_info;
    
    void start_interrupt(Interrupt interrupt_type);
} IrqManager;


#endif