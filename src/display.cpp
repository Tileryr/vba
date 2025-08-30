#include "src/scheduler.h"
#include "src/display.h"

Display::Display(SDL_Renderer * renderer, Memory * memory) : 
renderer(renderer), 
memory(memory), 
lcd_status(memory->memory_region(LCD_STATUS_ADDRESS)) {}

void Display::update_screen_bgmode_4() {
    Byte * vram = memory->memory_region(VRAM_START);
    Byte * background_palette = memory->memory_region(BG_PALETTE_RAM_START);

    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            HalfWord pixel_palette = vram[(y*SCREEN_WIDTH) + x] << 1;
            HalfWord palette_color = Memory::read_halfword_from_memory(background_palette, pixel_palette);
            screen[x][y] = palette_color;
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
    scheduler->schedule_event(VDRAW_CYCLE_LENGTH, [scheduler, this](){
        set_vblank(false);
        scheduler->schedule_event(VBLANK_CYCLE_LENGTH, [scheduler, this](){
            set_vblank(true);
            start_draw_loop(scheduler);
        });
    });
};

void Display::set_vblank(bool value) {
    Utils::write_bit(lcd_status, 0, value); 
}