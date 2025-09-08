#ifndef DISPLAY_INCLUDED
#define DISPLAY_INCLUDED

#include <SDL3/SDL.h>

#include "src/scheduler.h"
#include "src/memory.h"
#include "src/utils.h"

#include "src/cpu/cpu_types.h"
#include "src/cpu/bit_region.h"
#include "src/matrix.h"

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 160

#define MODE_5_SCREEN_WIDTH 160
#define MODE_5_SCREEN_HEIGHT 128

#define OAM_START 0x07000000
#define VRAM_START 0x06000000

#define BG_PALETTE_RAM_START 0x05000000
#define OBJ_PALETTE_RAM_START 0x05000200

#define DISPLAY_CONTROL_ADDRESS 0x04000000
#define DISPLAY_STATUS_ADDRESS 0x04000004
#define VCOUNT_ADDRESS 0x04000006

#define HDRAW_CYCLE_LENGTH 960U
#define HBLANK_CYCLE_LENGTH 272U

#define COLOR_TRANSPARENT 0xFFFF

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

    typedef struct AttributeZero {
        AttributeZero(HalfWord value);
        unsigned int y : 8;
        unsigned int object_mode : 2;
        unsigned int gfx_mode : 2;
        unsigned int mosiac : 1;
        unsigned int color_mode : 1;
        unsigned int sprite_shape : 2;
    } AttributeZero;

    typedef struct AttributeOne {
        AttributeOne(HalfWord value);
        unsigned int x : 9;
        
        unsigned int affine_index : 5;

        unsigned int horizontal_flip : 1;
        unsigned int vertical_flip : 1;

        unsigned int sprite_size : 2;
    } AttributeOne;

    typedef struct AttributeTwo {
        AttributeTwo(HalfWord value);
        unsigned int tile_index : 10;
        unsigned int priority : 2;
        unsigned int palette_bank : 4;
    } AttributeTwo;

    SDL_Renderer * renderer;
    Memory * memory;

    BitRegion vcount;

    Byte scanline;

    HalfWord screen[SCREEN_WIDTH][SCREEN_HEIGHT];

    void update_screen();
    void update_screen_bgmode_3();
    void update_screen_bgmode_4();
    void update_screen_bgmode_5();

    void update_sprites();
    void render_sprite(Byte sprite_number);

    void render_tile_4bpp(Matrix<HalfWord> * buffer, Byte charblock, HalfWord tile, Byte palbank, bool background_palette, HalfWord x, HalfWord y);
    void render_tile_8bpp(Byte charblock, HalfWord tile, Byte * palette_memory, HalfWord x, HalfWord y);

    void render();
    HalfWord get_palette_color(Byte index, bool background_palette);

    void start_draw_loop(Scheduler * scheduler);
} Display;

#endif