#include "src/scheduler.h"
#include "src/memory.h"
#include "src/display.h"
#include <functional>

Display::Display(SDL_Renderer * renderer, Memory * memory) : 
renderer(renderer), 
memory(memory), 

display_control(memory->memory_region(DISPLAY_CONTROL_ADDRESS)),
display_status(memory->memory_region(DISPLAY_STATUS_ADDRESS)),
vcount(memory->memory_region(VCOUNT_ADDRESS), 0, 7),

scanline(0),
tiled_backgrounds{
    TiledBackground(memory, 0),
    TiledBackground(memory, 1),
    TiledBackground(memory, 2),
    TiledBackground(memory, 3),
},

window_0(memory->memory_region(0x04000048), true, memory->memory_region(0x04000040),  memory->memory_region(0x04000044)),
window_1(memory->memory_region(0x04000048), false, memory->memory_region(0x04000042),  memory->memory_region(0x04000046)),
window_obj(memory->memory_region(0x0400004a), false),
window_outside(memory->memory_region(0x0400004a), true)
{}

void Display::update_screen() {
    memset(screen_buffers, 0xFF, sizeof(screen_buffers));

    switch (display_control.mode.get())
    {  
        case 0:
            update_screen_bgmode_0();
            break;
        case 1:
            update_screen_bgmode_1();
            break;
        case 2:
            update_screen_bgmode_2();
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

    if (display_control.display_objects.get()) {
        update_sprites();
    }
}

void Display::update_screen_bgmode_0() {
    for (int i=0;i<4;i++) {
        for (int y = 0; y < SCREEN_HEIGHT; y++) {
            render_tiled_background_scanline(tiled_backgrounds[i], y);
        }
    }
}

void Display::update_screen_bgmode_1() {
    render_tiled_background(tiled_backgrounds[0]);
    render_tiled_background(tiled_backgrounds[1]);
    render_tiled_background_affine(tiled_backgrounds[2]);
}

void Display::update_screen_bgmode_2() {
    render_tiled_background_affine(tiled_backgrounds[2]);
    render_tiled_background_affine(tiled_backgrounds[3]);
}

void Display::update_screen_bgmode_3() {
    Byte * vram = memory->memory_region(VRAM_START);
    
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            HalfWord color = Memory::read_halfword_from_memory(vram, (y*SCREEN_WIDTH*2) + (x*2))&(~0x8000);
            set_screen_pixel(x, y, color, BUFFER_BG2);
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
            HalfWord palette_index = vram[palette_address];

            set_screen_pixel(x, y, get_palette_color(palette_index, BG_PALETTE_RAM_START), BUFFER_BG2);
        }
    }
}

void Display::update_screen_bgmode_5() {
    Byte * vram = memory->memory_region(VRAM_START);
    
    for (int y = 0; y < MODE_5_SCREEN_HEIGHT; y++) {
        for (int x = 0; x < MODE_5_SCREEN_WIDTH; x++) {
            Word palette_address = ((y*MODE_5_SCREEN_WIDTH) + x)*2;
            if (display_control.display_frame_select.get() == 1) {
                palette_address += 0xA000;
            }

            HalfWord color = Memory::read_halfword_from_memory(vram, palette_address);
            set_screen_pixel(x, y, color, BUFFER_BG2);
        }
    }
}

void Display::update_sprites() {
    for (int i = 127; i >= 0; i--) {
        for (int y = 0; y < SCREEN_HEIGHT; y++) {
            render_sprite_scanline(i, y);
        }
    }
}

void Display::render_tiled_background_affine(TiledBackground background) {
    Word tile_size;
    switch (background.control.background_size.get()) {
        case 0:
            tile_size = 16;
            break;
        case 1:
            tile_size = 32;
            break;
        case 2:
            tile_size = 64;
            break;
        case 3:
            tile_size = 128;
            break;
    }
    Word pixel_size = tile_size*8;
    Matrix<HalfWord> background_buffer = Matrix<HalfWord>(pixel_size, pixel_size);
    Word screenblock_base_address = VRAM_START + (0x800*background.control.base_screenblock.get());
    Matrix<HalfWord>::iterate_xy(tile_size, tile_size, [&](Word tile_x, Word tile_y){
        Word screen_entry_index = ((tile_y*tile_size)+tile_x);
        
       
        Word screen_entry_address = screenblock_base_address+screen_entry_index;

        Word tile_index = memory->read_from_memory(screen_entry_address);
        Word tile_start_address = calculate_tile_start_address(background.control.base_charblock.get(), tile_index)*2;

        Word pixel_x = tile_x*8;
        Word pixel_y = tile_y*8;

        render_tile_8bpp(&background_buffer, tile_start_address, BG_PALETTE_RAM_START, pixel_x, pixel_y);
    });

    Matrix<int16_t> transformation = Matrix<int16_t>(2, 2);
    transformation.set(0, 0, background.affine_data.pa.get());
    transformation.set(1, 0, background.affine_data.pb.get());
    transformation.set(0, 1, background.affine_data.pc.get());
    transformation.set(1, 1, background.affine_data.pd.get());
    
    Matrix<HalfWord>::iterate_xy(SCREEN_WIDTH, SCREEN_HEIGHT, [&](Word x, Word y){
        Word mapped_pixel_x = (transformation.get(0, 0)*x + transformation.get(1, 0)*y);
        Word mapped_pixel_y = (transformation.get(0, 1)*x + transformation.get(1, 1)*y);
        mapped_pixel_x += background.affine_data.displacement_x.get();
        mapped_pixel_y += background.affine_data.displacement_y.get();
        mapped_pixel_x = mapped_pixel_x >> 8;
        mapped_pixel_y = mapped_pixel_y >> 8;
        
        if (background.control.affine_wrapping.get() == 1) {
            mapped_pixel_x %= pixel_size;
            mapped_pixel_y %= pixel_size;
        } 

        if (mapped_pixel_x > 0 && mapped_pixel_x < pixel_size && mapped_pixel_y > 0 && mapped_pixel_y < pixel_size) {
            HalfWord mapped_pixel_color = background_buffer.get(mapped_pixel_x, mapped_pixel_y);
            set_screen_pixel(x, y, mapped_pixel_color, number_to_bg_buffer_type(background.number));
        }
    });
}

void Display::render_tiled_background(TiledBackground background) {
    int tile_width;
    int tile_height;
    switch (background.control.background_size.get()) {
        case 0:
            tile_width = 32;
            tile_height = 32;
            break;
        case 1:
            tile_width = 64;
            tile_height = 32;
            break;
        case 2:
            tile_width = 32;
            tile_height = 64;
            break;
        case 3:
            tile_width = 64;
            tile_height = 64;
            break;
    }

    Word pixel_width = tile_width*8;
    Word pixel_height = tile_height*8;
    Matrix<HalfWord> background_buffer = Matrix<HalfWord>(pixel_width, pixel_height);

    Word screenblock_width = tile_width/32;
    Word screenblock_height = tile_height/32;
    
    Matrix<HalfWord>::iterate_xy(screenblock_width, screenblock_height, [&](Word screenblock_x, Word screenblock_y){
        Word screenblock_index = background.control.base_screenblock.get() + (screenblock_y*screenblock_width + screenblock_x);
        Word screenblock_base_address = VRAM_START + (0x800*screenblock_index);

        for (int y = 0; y < 32; y++) {
            for (int x = 0; x < 32; x++) {
                Word screen_entry_index = (y*32+x);
                Word screen_entry_address = screenblock_base_address+(screen_entry_index*2);
                TiledBackground::ScreenEntry screen_entry = TiledBackground::ScreenEntry(memory->memory_region(screen_entry_address));

                Word tile_start_address = calculate_tile_start_address(background.control.base_charblock.get(), screen_entry.tile_index.get());
                
                Word pixel_x = x*8 + screenblock_x*256;
                Word pixel_y = y*8 + screenblock_y*256;

                if (background.control.color_mode.get() == 0) {
                    render_tile_4bpp(
                        &background_buffer, 
                        tile_start_address,
                        BG_PALETTE_RAM_START,
                        screen_entry.palette_bank.get(),
                        pixel_x,
                        pixel_y
                    );
                } else {
                    SDL_Log("YADDA");
                    render_tile_8bpp(
                        &background_buffer, 
                        tile_start_address*2,
                        BG_PALETTE_RAM_START,
                        pixel_x,
                        pixel_y
                    );
                }       
            }
        }   
    });

    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            Word target_pixel_x = x+background.h_scroll.get();
            Word target_pixel_y = y+background.v_scroll.get();
            target_pixel_x %= pixel_width;
            target_pixel_y %= pixel_height;
            set_screen_pixel(x, y, background_buffer.get(target_pixel_x, target_pixel_y), number_to_bg_buffer_type(background.number));
        }
    }
}

void Display::render_tiled_background_scanline(TiledBackground background, int scanline) {
    int tile_width;
    int tile_height;
    switch (background.control.background_size.get()) {
        case 0:
            tile_width = 32;
            tile_height = 32;
            break;
        case 1:
            tile_width = 64;
            tile_height = 32;
            break;
        case 2:
            tile_width = 32;
            tile_height = 64;
            break;
        case 3:
            tile_width = 64;
            tile_height = 64;
            break;
    }

    Word h_offset = background.h_scroll.get();
    Word v_offset = background.v_scroll.get();

    Word pixel_width = tile_width*8;
    Word pixel_height = tile_height*8;

    int relative_scanline = scanline + v_offset;
    relative_scanline %= pixel_height;

    Word screenblock_width = tile_width/32;

    Word tile_y = (relative_scanline-(relative_scanline%8))/8;
    Word screenblock_y = (tile_y-(tile_y%32))/32;

    Word tile_offset_y = relative_scanline%8;

    for (int screenblock_x = 0; screenblock_x < screenblock_width; screenblock_x++) {
        Word screenblock_index = background.control.base_screenblock.get() + (screenblock_y*screenblock_width + screenblock_x);
        Word screenblock_base_address = VRAM_START + (0x800*screenblock_index);

        for (int tile_x = 0; tile_x < 32; tile_x++) {
            Word screen_entry_index = (tile_y*32+tile_x);

            Word screen_entry_address = screenblock_base_address+(screen_entry_index*2);
            TiledBackground::ScreenEntry screen_entry = TiledBackground::ScreenEntry(memory->memory_region(screen_entry_address));

            Word base_charblock = background.control.base_charblock.get();
            Word tile_index = screen_entry.tile_index.get();

            for (int tile_pixel_x = 0; tile_pixel_x < 8; tile_pixel_x++) {
                Word screen_x = tile_pixel_x + tile_x*8 + screenblock_x*256 + -background.h_scroll.get();
                screen_x %= pixel_width;

                HalfWord background_color = get_tile_pixel_4bpp(
                    tile_pixel_x,
                    tile_offset_y,
                    tile_index,
                    base_charblock,
                    BG_PALETTE_RAM_START,
                    screen_entry.palette_bank.get(),
                    8
                );

                set_screen_pixel(screen_x, scanline, background_color, number_to_bg_buffer_type(background.number));
            }
        }
    }
}

void Display::render_sprite_scanline(Byte sprite_number, int y) {
    Byte * oam = memory->memory_region(OAM_START);
    Byte * object_attributes_start = &oam[sprite_number * 8];

    HalfWord attribute_0_value = Memory::read_halfword_from_memory(object_attributes_start, 0);
    AttributeZero attribute_0 = AttributeZero(attribute_0_value);

    HalfWord attribute_1_value = Memory::read_halfword_from_memory(object_attributes_start, 2);
    AttributeOne attribute_1 = AttributeOne(attribute_1_value);

    HalfWord attribute_2_value = Memory::read_halfword_from_memory(object_attributes_start, 4);
    AttributeTwo attribute_2 = AttributeTwo(attribute_2_value);

    if (attribute_0.object_mode == 0b10) {
        return;
    }

    HalfWord pixel_size_x;
    HalfWord pixel_size_y;
    HalfWord base_pixel_size;
    switch (attribute_1.sprite_size)
    {
        case 0b00:
            base_pixel_size = 8;
            break;
        case 0b01:
            base_pixel_size = 16;
            break;
        case 0b10:
            base_pixel_size = 32;
            break;
        case 0b11:
            base_pixel_size = 64;
            break;
    }

    if (attribute_0.sprite_shape != 0b00) {
        if (attribute_1.sprite_size == 0b01) {
            pixel_size_x = 32;
            pixel_size_y = 8;
        } else {
            pixel_size_x = base_pixel_size;
            pixel_size_y = base_pixel_size/2;

            if (attribute_1.sprite_size == 0b00) {
                pixel_size_x *= 2;
                pixel_size_y *= 2;
            }
        }
        
        if (attribute_0.sprite_shape == 0b10) {
            std::swap(pixel_size_x, pixel_size_y);
        }
    } else {
        pixel_size_x = base_pixel_size;
        pixel_size_y = base_pixel_size;
    }

    bool affine = attribute_0.object_mode == 0b01 || attribute_0.object_mode == 0b11;
    bool double_sized = attribute_0.object_mode == 0b11;

    Word y_max_limit;
    if (affine && double_sized) {
        y_max_limit = attribute_0.y+(pixel_size_y*2);
    } else {
        y_max_limit = attribute_0.y+pixel_size_y;
    }

    if (attribute_0.y + pixel_size_y > 255) {
        if (y < attribute_0.y && y >= y_max_limit%256) {  
            return;
        }
    } else {
        if (y < attribute_0.y || y >= y_max_limit) {
            return;
        }
    }

    Word y_offset_from_base = y - attribute_0.y;
    if (y < attribute_0.y) {
        y_offset_from_base = (y + 256) - attribute_0.y; 
    }

    if (attribute_1.vertical_flip == 1) {
        y_offset_from_base = flip(y_offset_from_base, pixel_size_y/2);
    }

    Matrix<HalfWord> sprite_scanline_buffer = Matrix<HalfWord>(pixel_size_x, 1);

    HalfWord tile_size_x = pixel_size_x / 8;
    HalfWord tile_size_y = pixel_size_y / 8;
    
    auto get_pixel_4bpp = [&](Word x, Word y){
        return get_tile_pixel_4bpp(
            x, 
            y, 
            attribute_2.tile_index, 
            4, 
            OBJ_PALETTE_RAM_START,
            attribute_2.palette_bank, 
            pixel_size_x
        );
    };

    if (affine) {
        Matrix<int16_t> transform_matrix = Matrix<int16_t>(2, 2);
        transform_matrix.for_each([&](Word x, Word y){
            Word index = (y*2+x+1);
            transform_matrix.set(x, y, memory->read_halfword_from_memory(OAM_START + ((4*index)-1)*2 + attribute_1.affine_index*32)); 
        });

        int32_t half_sprite_width = pixel_size_x/2;
        int32_t half_sprite_height = pixel_size_y/2;

        int32_t render_area_width = half_sprite_width;
        int32_t render_area_height = half_sprite_height;

        if (double_sized) {
            render_area_width *= 2;
            render_area_height *= 2;
        }

        Word base_x = attribute_1.x+render_area_width;
        Word base_y = attribute_0.y+render_area_height;

        int relative_y = y - base_y;

        for (int x = -render_area_width; x < render_area_width; x++) {
            int16_t mapped_pixel_x = (transform_matrix.get(0, 0)*x + transform_matrix.get(1, 0)*relative_y)>>8;
            int16_t mapped_pixel_y = (transform_matrix.get(0, 1)*x + transform_matrix.get(1, 1)*relative_y)>>8;

            Word target_sprite_pixel_x = mapped_pixel_x+half_sprite_width;
            Word target_sprite_pixel_y = mapped_pixel_y+half_sprite_height;
            if (target_sprite_pixel_x < 0 || target_sprite_pixel_x >= pixel_size_x || target_sprite_pixel_y < 0 || target_sprite_pixel_y >= pixel_size_y) {continue;}

            HalfWord mapped_pixel_color = get_pixel_4bpp(target_sprite_pixel_x, target_sprite_pixel_y);

            Word screen_x = base_x+x;
            Word screen_y = base_y+relative_y;
            screen_x %= 512;
            screen_y %= 256;
            set_screen_pixel(screen_x, screen_y, mapped_pixel_color, BUFFER_SPIRTE);
        }
    } else {
        for (int x = 0; x < pixel_size_x; x++) {
            Word mapped_x = x;
            if (attribute_1.horizontal_flip == 1) {
                mapped_x = flip(mapped_x, pixel_size_x/2);
            }

            HalfWord pixel_color = get_pixel_4bpp(mapped_x, y_offset_from_base);

            Word screen_x = attribute_1.x+x;
            screen_x %= 512;
            set_screen_pixel(screen_x, y, pixel_color, BUFFER_SPIRTE);
        }
    }
}

void Display::render_tile_4bpp(Matrix<HalfWord> * buffer, Word tile_start_address, Word palette_start_address, Byte palbank, HalfWord x, HalfWord y) {
    Byte * vram = memory->memory_region(VRAM_START);

    for (int offset_y = 0; offset_y < 8; offset_y++) {
        for (int offset_x = 0; offset_x < 4; offset_x += 1) {
            Byte palette_index_low;
            Byte palette_index_high;

            palette_index_low = vram[tile_start_address + offset_x + (offset_y * 4)] & 0xF;
            palette_index_high = (vram[tile_start_address + offset_x + (offset_y * 4)] >> 4) & 0xF;

            palette_index_low |= (palbank << 4);
            palette_index_high |= (palbank << 4);

            Word buffer_x = offset_x*2 + x;
            Word buffer_y = offset_y + y;

            HalfWord low_palette_color = get_palette_color(palette_index_low, palette_start_address);
            HalfWord high_palette_color = get_palette_color(palette_index_high, palette_start_address);

            buffer->set(buffer_x, buffer_y, low_palette_color);
            buffer->set(buffer_x+1, buffer_y, high_palette_color);
        }
    }
}

HalfWord Display::get_tile_pixel_4bpp(Word x, Word y, Word tile_base, Word charblock, Word palette_start, Word palette_bank, Word width) {
    Word tile_x, tile_y;
    tile_x = (x - (x % 8))/8;
    tile_y = (y - (y % 8))/8;

    bool one_dimensional = display_control.vram_mapping.get() == 1;
    Word tile_index;
    if (one_dimensional) {
        Word tile_width = (width/8);
        tile_index = tile_base + (tile_x + tile_y*tile_width);
    } else {
        tile_index = tile_base + (tile_x + tile_y*0x20);
    }

    Word tile_start_address = (charblock * 0x4000) + tile_index * 0x20;

    Byte * vram = memory->memory_region(VRAM_START);
    Word x_offset = x % 8;
    Word y_offset = y % 8;

    Byte palette_index;
    palette_index = vram[tile_start_address + (x_offset/2) + (y_offset * 4)];

    if (x % 2 == 1) {
        palette_index = (palette_index >> 4) & 0xF;
    } else {
        palette_index = palette_index & 0xF;
    }

    palette_index |= (palette_bank << 4);
    HalfWord palette_color = get_palette_color(palette_index, palette_start);

    return palette_color;
}

void Display::render_tile_8bpp(Matrix<HalfWord> * buffer, Word tile_start_address, Word palette_start_address, HalfWord x, HalfWord y) {
    Byte * vram = memory->memory_region(VRAM_START);

    for (int offset_y = 0; offset_y < 8; offset_y++) {
        for (int offset_x = 0; offset_x < 8; offset_x += 1) {
            Byte palette_index;

            palette_index = vram[tile_start_address + offset_x + (offset_y * 8)];

            Word buffer_x = offset_x + x;
            Word buffer_y = offset_y + y;

            buffer->set(buffer_x, buffer_y, get_palette_color(palette_index, palette_start_address));
        }
    }
}

// void Display::render_tile_scanline_8bpp(Matrix<HalfWord> * buffer, Word tile_start_address, Word palette_start_address, HalfWord x, HalfWord y, int offset_y) {
//     Byte * vram = memory->memory_region(VRAM_START);

//     for (int offset_x = 0; offset_x < 8; offset_x += 1) {
//         Byte palette_index;

//         palette_index = vram[tile_start_address + offset_x + (offset_y * 8)];

//         Word buffer_x = offset_x + x;
//         Word buffer_y = offset_y + y;

//         buffer->set(buffer_x, buffer_y, get_palette_color(palette_index, palette_start_address));
//     }
// }

void Display::render() {
    SDL_RenderClear(renderer);

    bool window_0_active = display_control.display_window_0.get();
    bool window_1_active = display_control.display_window_1.get();
    bool window_obj_active = display_control.display_objects_window.get();

    bool windows_active = window_0_active || window_1_active || window_obj_active;

    RenderSettings settings;
    settings.bg0 = true;
    settings.bg1 = true;
    settings.bg2 = true;
    settings.bg3 = true;
    settings.obj = true;
    settings.sfx = true;

    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            HalfWord color = memory->read_halfword_from_memory(BG_PALETTE_RAM_START);
            HalfWord sprite_color = screen_buffers[BUFFER_SPIRTE][x][y];

            if (windows_active) {
                window_0.update_inside(x, y);
                window_1.update_inside(x, y);
                window_obj.inside = sprite_color != COLOR_TRANSPARENT;
                window_outside.inside = (!window_0.inside) && (!window_1.inside) && (!window_obj.inside);

                settings = window_outside.get_render_settings();
                if (window_obj.inside && window_obj_active) settings = window_obj.get_render_settings();
                if (window_1.inside   && window_1_active)   settings = window_1.get_render_settings();
                if (window_0.inside   && window_0_active)   settings = window_0.get_render_settings();
            }

            bool sprites_enabled = display_control.display_objects.get() == 1;
            if (sprite_color != COLOR_TRANSPARENT && sprites_enabled && settings.obj) {
                color = sprite_color;
            } else {
                Word backgrounds_displayed = display_control.display_backgrounds.get();
                for (int i = 0; i < 4; i++) {
                    HalfWord background_color = screen_buffers[i+1][x][y];

                    bool background_setting;
                    bool background_enabled = Utils::read_bit(backgrounds_displayed, i);

                    switch (i) {
                        case 0:
                            background_setting = settings.bg0;
                            break;
                        case 1:
                            background_setting = settings.bg1;
                            break;
                        case 2:
                            background_setting = settings.bg2;
                            break;
                        case 3:
                            background_setting = settings.bg3;
                            break;
                    }

                    if (background_color != COLOR_TRANSPARENT && background_enabled && background_setting) {
                        
                        color = background_color;
                        break;
                    }
                }
            }
            
            Byte r = Utils::read_bit_range(color, 0,  4);
            Byte g = Utils::read_bit_range(color, 5,  9);
            Byte b = Utils::read_bit_range(color, 10, 14);

            SDL_SetRenderDrawColor(renderer, r << 3, g << 3, b << 3, SDL_ALPHA_OPAQUE);
            SDL_RenderPoint(renderer, x, y);
        }
    }
    
    SDL_RenderPresent(renderer);
}

void Display::set_screen_pixel(Word x, Word y, HalfWord color, BufferType buffer) {
    if (color == COLOR_TRANSPARENT) {return;}
    if (x >= SCREEN_WIDTH) {return;}
    if (y >= SCREEN_HEIGHT) {return;}

    screen_buffers[buffer][x][y] = color;
}

HalfWord Display::get_palette_color(Byte index, Word palatte_start_address) {
    Byte * palette = memory->memory_region(palatte_start_address);
    if (index % 16 == 0) {
        return COLOR_TRANSPARENT;
    } 
    return Memory::read_halfword_from_memory(palette, index*2) & (~0x8000);
}

Word Display::calculate_tile_start_address(Word charblock, Word tile) {
    return (charblock * 0x4000) + tile * 0x20;
}

Word Display::flip(Word number, Word flip_value) {
    return (-(number-flip_value))+(flip_value-1);
}

Display::BufferType Display::number_to_bg_buffer_type(int number) {
    switch (number)
    {
        case 0: return BUFFER_BG0;
        case 1: return BUFFER_BG1;
        case 2: return BUFFER_BG2;
        case 3: return BUFFER_BG3;
    }

    SDL_assert(false);
    return BUFFER_BG0;
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
display_window_0(memory_location, 13),
display_window_1(memory_location, 14),
display_objects_window(memory_location, 15)
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

Display::TiledBackground::TiledBackground(Memory * memory, Byte number) :
control(memory->memory_region(0x04000008+(2*number))),
h_scroll(memory->memory_region(0x04000010+(4*number)), 0, 15),
v_scroll(memory->memory_region(0x04000012+(4*number)), 0, 15),
affine_data(memory->memory_region(0x04000020+(0x10*(number-2)))),
number(number)
{}

Display::TiledBackground::TiledBackgroundControl::TiledBackgroundControl(Byte * address) : 
priority(address,0,1),
base_charblock(address,2,3),
mosiac(address,6),
color_mode(address,7),
base_screenblock(address,8,12),
affine_wrapping(address,13),
background_size(address,14,15)
{}

Display::TiledBackground::AffineData::AffineData(Byte * address) : 
pa(address, 0, 15),
pb(address+0x2, 0, 15),
pc(address+0x4, 0, 15),
pd(address+0x6, 0, 15),
displacement_x(address+0x8, 0, 31),
displacement_y(address+0xc, 0, 31)
{}

Display::TiledBackground::ScreenEntry::ScreenEntry(Byte * address) : 
tile_index(address, 0, 9),
horizontal_flip(address, 10),
vertical_flip(address, 11),
palette_bank(address, 12, 15)
{}

Display::Window::Window(Byte * content_address, bool start) :
bg0(content_address, start ? 0 : 8),
bg1(content_address, start ? 1 : 9),
bg2(content_address, start ? 2 : 10),
bg3(content_address, start ? 3 : 11),
obj(content_address, start ? 4 : 12),
sfx(content_address, start ? 5 : 13),
inside(false) {}

Display::RenderSettings Display::Window::get_render_settings() {
    return {
        .bg0 = bg0.get() != 0,
        .bg1 = bg1.get() != 0,
        .bg2 = bg2.get() != 0,
        .bg3 = bg3.get() != 0,
        .obj = obj.get() != 0,
        .sfx = sfx.get() != 0,
    };
}

Display::SizableWindow::SizableWindow(Byte * content_address, bool start, Byte * h_address, Byte * v_address) : 
Window(content_address, start),
right(h_address, 0, 7),
left(h_address, 8, 15),
bottom(v_address, 0, 7),
top(v_address, 8, 15),
inside_h(false),
inside_v(false)
{}

void Display::SizableWindow::update_inside(Word x, Word y) {
    if (y == top.get()) inside_v = true;
    if (y == bottom.get()) inside_v = false;

    if (x == left.get()) inside_h = true;
    if (x == right.get()) inside_h = false;

    inside = inside_h && inside_v;
}

Display::AttributeZero::AttributeZero(HalfWord value) {
    y = Utils::read_bit_range(value, 0, 7);
    object_mode = Utils::read_bit_range(value, 8, 9);
    gfx_mode = Utils::read_bit_range(value, 0xA, 0xB);
    mosiac = Utils::read_bit(value, 0xC);
    color_mode = Utils::read_bit(value, 0xD);
    sprite_shape = Utils::read_bit_range(value, 0xE, 0xF);
}

Display::AttributeOne::AttributeOne(HalfWord value) {
    x = Utils::read_bit_range(value, 0, 8);
    affine_index = Utils::read_bit_range(value, 9, 0xD);
    horizontal_flip = Utils::read_bit(value, 0xC);
    vertical_flip = Utils::read_bit(value, 0xD);
    sprite_size = Utils::read_bit_range(value, 0xE, 0xF);
}

Display::AttributeTwo::AttributeTwo(HalfWord value) {
    tile_index = Utils::read_bit_range(value, 0, 9);
    priority = Utils::read_bit_range(value, 0xA, 0xB);
    palette_bank = Utils::read_bit_range(value, 0xC, 0xF);
}