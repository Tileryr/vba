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
        BitRegion display_window_0;
        BitRegion display_window_1;
        BitRegion display_objects_window;
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
        AttributeZero(Byte * oam_start);
        BitRegion y;
        BitRegion object_mode;
        BitRegion gfx_mode;
        BitRegion mosiac;
        BitRegion color_mode;
        BitRegion sprite_shape;
    } AttributeZero;

    typedef struct AttributeOne {
        AttributeOne(Byte * oam_start);
        BitRegion x;
        BitRegion affine_index;

        BitRegion horizontal_flip;
        BitRegion vertical_flip;

        BitRegion sprite_size;
    } AttributeOne;

    typedef struct AttributeTwo {
        AttributeTwo(Byte * oam_start);
        BitRegion tile_index;
        BitRegion priority;
        BitRegion palette_bank;
    } AttributeTwo;

    struct Sprite {
        Sprite(Memory * memory, int number);
        AttributeZero attribute_0;
        AttributeOne attribute_1;
        AttributeTwo attribute_2;
    } * sprites;

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

        struct AffineData {
            AffineData(Byte * address);
            BitRegion pa, pb, pc, pd;
            BitRegion displacement_x, displacement_y;
        } affine_data;

        typedef struct ScreenEntry {
            ScreenEntry(Byte * address);
            BitRegion tile_index;
            BitRegion horizontal_flip;
            BitRegion vertical_flip;
            BitRegion palette_bank;
        } ScreenEntry;
        
        BitRegion h_scroll;
        BitRegion v_scroll;
        int number;
    } TiledBackground;
    
    typedef struct RenderSettings {
        bool bg0;
        bool bg1;
        bool bg2;
        bool bg3;
        bool obj;
        bool sfx;
    } RenderSettings;

    typedef struct Window {
        Window(Byte * content_address, bool start);
        BitRegion bg0;
        BitRegion bg1;
        BitRegion bg2;
        BitRegion bg3;
        BitRegion obj;
        BitRegion sfx;

        RenderSettings get_render_settings();
        bool inside;
    } Window;

    typedef struct SizableWindow : Window {
        SizableWindow(Byte * content_address, bool start, Byte * h_address, Byte * v_address);
        BitRegion left;
        BitRegion right;
        BitRegion top;
        BitRegion bottom;

        bool inside_h;
        bool inside_v;
        void update_inside(Word x, Word y);
    } SizableWindow;


    SizableWindow window_0;
    SizableWindow window_1;
    Window window_obj;
    Window window_outside;
    
    TiledBackground tiled_backgrounds[4];

    SDL_Renderer * renderer;
    Memory * memory;

    BitRegion vcount;

    Byte scanline;

    enum BufferType {
        BUFFER_SPIRTE=0,
        BUFFER_BG0=1,
        BUFFER_BG1=2,
        BUFFER_BG2=3,
        BUFFER_BG3=4,
    };

    HalfWord screen_buffers[5][SCREEN_WIDTH][SCREEN_HEIGHT];

    void update_scanline(int y);

    void update_scanline_bgmode_0(int y);
    void update_scanline_bgmode_1(int y);
    void update_scanline_bgmode_2(int y);
    void update_scanline_bgmode_3(int y);
    void update_scanline_bgmode_4(int y); 
    void update_scanline_bgmode_5(int y);

    HalfWord get_tile_pixel_4bpp(Word x, Word y, Word tile_index, Word charblock, Word palette_start, Word palette_bank, Word sprite_width);
    HalfWord get_tile_pixel_8bpp(Word x, Word y, Word tile_index, Word charblock, Word palette_start, Word sprite_width);
    
    void render_tiled_background_affine_scanline(TiledBackground background, int y);
    void render_tiled_background_scanline(TiledBackground background, int y);

    void render_sprite_scanline(Sprite sprite, int y);

    inline Word flip(Word number, Word flip_value);

    void render();
    
    void set_screen_pixel(Word x, Word y, HalfWord color, BufferType buffer);
    HalfWord get_palette_color(Byte index, Word palette_start_address);
    BufferType number_to_bg_buffer_type(int number);

    void start_draw_loop(Scheduler * scheduler);
} Display;

#endif