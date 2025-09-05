#ifndef DISPLAY_INCLUDED
#define DISPLAY_INCLUDED

#include <SDL3/SDL.h>

#include "src/scheduler.h"
#include "src/memory.h"
#include "src/utils.h"

#include "src/cpu/cpu_types.h"
#include "src/cpu/bit_region.h"

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 160

#define VRAM_START 0x06000000

#define BG_PALETTE_RAM_START 0x05000000
#define OBJ_PALETTE_RAM_START 0x05000200

#define DISPLAY_CONTROL_ADDRESS 0x04000000
#define DISPLAY_STATUS_ADDRESS 0x04000004
#define VCOUNT_ADDRESS 0x04000006

#define HDRAW_CYCLE_LENGTH 960U
#define HBLANK_CYCLE_LENGTH 272U

typedef struct Display {
    Display(SDL_Renderer * renderer, Memory * memory);

    struct DisplayControl {
        DisplayControl(Byte * memory_location);
        BitRegion mode;
        BitRegion display_frame_select;
        BitRegion hblank_interval_free;
        BitRegion vram_mapping;
        BitRegion forced_blank;
        BitRegion display_backgrounds;
        BitRegion display_objects;
        BitRegion display_windows;
        BitRegion display_objects_windows;
    } display_control;

    struct DisplayStatus {
        DisplayStatus(Byte * memory_location);
        BitRegion vblank;
        BitRegion hblank;
        BitRegion vcount;
        BitRegion vblank_irq;
        BitRegion hblank_irq;
        BitRegion vcount_irq;
        BitRegion vcount_setting;
    } display_status;

    SDL_Renderer * renderer;
    Memory * memory;

    BitRegion vcount;

    Byte scanline;


    HalfWord screen[SCREEN_WIDTH][SCREEN_HEIGHT];

    void update_screen_bgmode_4();

    void render();

    void start_draw_loop(Scheduler * scheduler);
} Display;

#endif