#ifndef DISPLAY_INCLUDED
#define DISPLAY_INCLUDED

#include <SDL3/SDL.h>
#include "src/scheduler.h"
#include "src/memory.h"
#include "src/utils.h"
#include "src/cpu/cpu_types.h"

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 160

#define VRAM_START 0x06000000

#define BG_PALETTE_RAM_START 0x05000000
#define OBJ_PALETTE_RAM_START 0x05000200

#define LCD_STATUS_ADDRESS 0x04000004

#define PIXEL_CYCLE_LENGTH 4
#define HDRAW_CYCLE_LENGTH 960
#define HBLANK_CYCLE_LENGTH 272
#define SCANLINE_CYCLE_LENGTH 1232
#define VDRAW_CYCLE_LENGTH 197120
#define VBLANK_CYCLE_LENGTH 83776
#define REFRESH_CYCLE_LENGTH 280896

typedef struct Display {
    Display(SDL_Renderer * renderer, Memory * memory);

    SDL_Renderer * renderer;

    Memory * memory;
    Byte * lcd_status;

    HalfWord screen[SCREEN_WIDTH][SCREEN_HEIGHT];

    void update_screen_bgmode_4();

    void render();

    void start_draw_loop(Scheduler * scheduler);
    void set_vblank(bool value);
} Display;

#endif