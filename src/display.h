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

    typedef struct TiledBackground {
        TiledBackground(Memory * memory, Byte number);
        struct TiledBackgroundControl {
            TiledBackgroundControl(Byte * address);
            BitRegion priority;
            BitRegion base_charblock;
            BitRegion mosiac;
            BitRegion color_mode;
            BitRegion base_screenblock;
            BitRegion affine_wrapping;
            BitRegion background_size;
        } control;

        typedef struct ScreenEntry {
            ScreenEntry(Byte * address);
            BitRegion tile_index;
            BitRegion horizontal_flip;
            BitRegion vertical_flip;
            BitRegion palette_bank;
        } ScreenEntry;
        
        BitRegion h_scroll;
        BitRegion v_scroll;
    } TiledBackground;
    
    TiledBackground tiled_backgrounds[4];

    SDL_Renderer * renderer;
    Memory * memory;

    BitRegion vcount;

    Byte scanline;

    HalfWord screen[SCREEN_WIDTH][SCREEN_HEIGHT];

    void update_screen();
    void update_screen_bgmode_0();
    void update_screen_bgmode_1();
    void update_screen_bgmode_2();
    void update_screen_bgmode_3();
    void update_screen_bgmode_4();
    void update_screen_bgmode_5();

    void update_sprites();

    void render_tiled_background_affine(TiledBackground background_number);
    void render_tiled_background(TiledBackground background);
    void render_sprite(Byte sprite_number);

    void render_tile_4bpp(Matrix<HalfWord> * buffer, Word tile_start_address, Word palette_start_address, Byte palbank, HalfWord x, HalfWord y);
    void render_tile_8bpp(Matrix<HalfWord> * buffer, Word tile_start_address, Word palette_start_address, HalfWord x, HalfWord y);

    void apply_affine_transformation_sprite(Matrix<HalfWord> * sprite, Matrix<int16_t> * transformation, Word sprite_x, Word sprite_y, bool double_render_area);
    inline Word calculate_tile_start_address(Word charblock, Word tile);

    void render();
    
    void set_screen_pixel(Word x, Word y, HalfWord color);
    HalfWord get_palette_color(Byte index, Word palette_start_address);

    void start_draw_loop(Scheduler * scheduler);
} Display;

#endif