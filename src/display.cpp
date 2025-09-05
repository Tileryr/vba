#include "src/scheduler.h"
#include "src/memory.h"
#include "src/display.h"

Display::Display(SDL_Renderer * renderer, Memory * memory) : 
renderer(renderer), 
memory(memory), 

display_control(memory->memory_region(DISPLAY_CONTROL_ADDRESS)),
display_status(memory->memory_region(DISPLAY_STATUS_ADDRESS)),
vcount(memory->memory_region(VCOUNT_ADDRESS), 0, 7),

scanline(0)
{}

void Display::update_screen() {
    switch (display_control.mode.get())
    {
        case 0:
            break;
        case 1:
            break;
        case 2:
            break;
        case 3:
            update_screen_bgmode_3();
            break;
        case 4:
            update_screen_bgmode_4();
            break;
        case 5:
            update_screen_bgmode_5();
            break;
    }
}

void Display::update_screen_bgmode_3() {
    Byte * vram = memory->memory_region(VRAM_START);
    
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            HalfWord color = Memory::read_halfword_from_memory(vram, (y*SCREEN_WIDTH*2) + (x*2));
            screen[x][y] = color;
        }
    }
}

void Display::update_screen_bgmode_4() {
    Byte * vram = memory->memory_region(VRAM_START);
    Byte * background_palette = memory->memory_region(BG_PALETTE_RAM_START);

    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            Word palette_address =(y*SCREEN_WIDTH) + x;
            if (display_control.display_frame_select.get() == 1) {
                palette_address += 0xA000;
            }
            HalfWord pixel_palette = vram[palette_address] << 1;
            

            HalfWord palette_color = Memory::read_halfword_from_memory(background_palette, pixel_palette);
            screen[x][y] = palette_color;
        }
    }
}

void Display::update_screen_bgmode_5() {
    Byte * vram = memory->memory_region(VRAM_START);
    memset(screen, 0, sizeof(screen));

    for (int y = 0; y < MODE_5_SCREEN_HEIGHT; y++) {
        for (int x = 0; x < MODE_5_SCREEN_WIDTH; x++) {
            Word palette_address = (y*MODE_5_SCREEN_WIDTH*2) + x*2;
            if (display_control.display_frame_select.get() == 1) {
                palette_address += 0xA000;
            }
            HalfWord color = Memory::read_halfword_from_memory(vram, palette_address);
            screen[x][y] = color;
        }
    }
}

void Display::render() {
    // MODE 4
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);

    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            HalfWord color = screen[x][y];

            Byte r = Utils::read_bit_range(color, 0,  4);
            Byte g = Utils::read_bit_range(color, 5,  9);
            Byte b = Utils::read_bit_range(color, 10, 14);

            SDL_SetRenderDrawColor(renderer, r << 3, g << 3, b << 3, SDL_ALPHA_OPAQUE);
            SDL_RenderPoint(renderer, x, y);
        }
    }
    
    SDL_RenderPresent(renderer);
}

void Display::start_draw_loop(Scheduler * scheduler) {
    scanline++;

    if (scanline > 226) {
        display_status.vblank.set(false);
        scanline = 0;
    } else if (scanline >= 160) {
        display_status.vblank.set(true);
        if (display_status.vblank_irq.get() == true) {
            // DO IRQ   
        }
    }

    if (display_status.vcount_irq.get() == true && scanline == display_status.vcount_setting.get()) {
        // DO IRQ   
    }
    
    vcount.set(scanline);

    scheduler->schedule_event(HDRAW_CYCLE_LENGTH, [scheduler, this](){
        display_status.hblank.set(true);
        if (display_status.hblank_irq.get() == true) {
            // DO IRQ   
        }
        scheduler->schedule_event(HBLANK_CYCLE_LENGTH, [scheduler, this](){
            display_status.hblank.set(false);
            start_draw_loop(scheduler);
        });
    });
};

Display::DisplayControl::DisplayControl(Byte * memory_location) :
mode(memory_location, 0, 2),
display_frame_select(memory_location, 4),
hblank_interval_free(memory_location, 5),
vram_mapping(memory_location, 6),
forced_blank(memory_location, 7),
display_backgrounds(memory_location, 8, 11),
display_objects(memory_location, 12),
display_windows(memory_location, 13, 14),
display_objects_windows(memory_location, 15)
{}

Display::DisplayStatus::DisplayStatus(Byte * memory_location) :
vblank(memory_location, 0),
hblank(memory_location, 1),
vcount(memory_location, 2),
vblank_irq(memory_location, 3),
hblank_irq(memory_location, 4),
vcount_irq(memory_location, 5),
vcount_setting(memory_location, 8, 15)
{}