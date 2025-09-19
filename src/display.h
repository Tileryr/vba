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

    HalfWord get_sprite_pixel_4bpp(Word x, Word y, Word tile_index, Word charblock, Word palette_start, Word palette_bank, Word sprite_width);

    void update_scanline(int y);

    void render_tiled_background_affine_scanline(TiledBackground background_number, int y);
    void render_tiled_background_scanline(TiledBackground background, int y);
    void render_sprite_scanline(Byte sprite_number, int y);

    Word flip(Word number, Word flip_value);

    void render_tile_scanline_4bpp(Matrix<HalfWord> * buffer, Word tile_start_address, Word palette_start_address, Byte palbank, HalfWord x, HalfWord y, int offset_y);
    void render_tile_scanline_8bpp(Matrix<HalfWord> * buffer, Word tile_start_address, Word palette_start_address, HalfWord x, HalfWord y, int offset_y);
    
    void apply_affine_transformation_sprite(Matrix<HalfWord> * sprite, Matrix<int16_t> * transformation, Word sprite_x, Word sprite_y, bool double_render_area);
    inline Word calculate_tile_start_address(Word charblock, Word tile);

    void render();
    
    void set_screen_pixel(Word x, Word y, HalfWord color, BufferType buffer);
    HalfWord get_palette_color(Byte index, Word palette_start_address);
    BufferType number_to_bg_buffer_type(int number);

    void start_draw_loop(Scheduler * scheduler);
} Display;

#endif