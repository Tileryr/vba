#include "src/scheduler.h"
#include "src/memory.h"
#include "src/display.h"

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
}
{}

void Display::update_screen() {
    memset(screen, 0, sizeof(screen));

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

    update_sprites();
}

void Display::update_screen_bgmode_0() {
    Byte display_backgrounds = display_control.display_backgrounds.get();
    for (int i=0;i<4;i++) {
        if (!Utils::read_bit(display_backgrounds, i)) {continue;}
        render_tiled_background(tiled_backgrounds[i]);
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
            HalfWord palette_index = vram[palette_address];

            screen[x][y] = get_palette_color(palette_index, true);
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
            screen[x][y] = color;
        }
    }
}

void Display::update_sprites() {
    for (int i = 0; i < 128; i++) {
        render_sprite(i);
    }
}

void Display::render_tiled_background_affine(TiledBackground background_number) {}
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
            screen[x][y] = background_buffer.get(target_pixel_x, target_pixel_y);
        }
    }
}

void Display::render_sprite(Byte sprite_number) {
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

    Matrix<HalfWord> sprite_buffer = Matrix<HalfWord>(pixel_size_x, pixel_size_y);

    HalfWord tile_size_x = pixel_size_x / 8;
    HalfWord tile_size_y = pixel_size_y / 8;

    for (int y = 0; y < tile_size_y; y++) {
        for (int x = 0; x < tile_size_x; x++) {
            bool one_dimensional = display_control.vram_mapping.get() == 1;
            Word tile_x = x*8;
            Word tile_y = y*8;
            Word tile_index;

            if (one_dimensional) {
                tile_index = attribute_2.tile_index + (y*tile_size_x + x);
            } else {
                tile_index = attribute_2.tile_index + (y*0x20 + x);
            }

            if (attribute_0.color_mode == 0) {
                render_tile_4bpp(
                    &sprite_buffer,
                    calculate_tile_start_address(4, tile_index),
                    OBJ_PALETTE_RAM_START,
                    attribute_2.palette_bank,
                    tile_x,
                    tile_y
                );
            } else {
                render_tile_8bpp(
                    &sprite_buffer,
                    calculate_tile_start_address(4, tile_index),
                    OBJ_PALETTE_RAM_START,
                    tile_x,
                    tile_y
                );
            }   
        }
    }

    if (attribute_1.horizontal_flip == 1) {
        Matrix<HalfWord> hflip_buffer = Matrix<HalfWord>(pixel_size_x, pixel_size_y);
        sprite_buffer.for_each([&hflip_buffer, &sprite_buffer](Word x, Word y) mutable {
            hflip_buffer.set(x, y, sprite_buffer.get((-(x-32))+31, y));
        });

        sprite_buffer.copy(&hflip_buffer);
    }
    
    if (attribute_1.vertical_flip == 1) {
        Matrix<HalfWord> vflip_buffer = Matrix<HalfWord>(pixel_size_x, pixel_size_y);
        sprite_buffer.for_each([&vflip_buffer, &sprite_buffer](Word x, Word y) mutable {
            vflip_buffer.set(x, y, sprite_buffer.get(x, (-(y-32))+31));
        });

        sprite_buffer.copy(&vflip_buffer);
    } 

    sprite_buffer.for_each([&](Word x, Word y){
        screen[attribute_1.x+x][attribute_0.y+y] = sprite_buffer.get(x, y);
    });
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
void Display::render_tile_8bpp(Matrix<HalfWord> * buffer, Word tile_start_address, Word palette_start_address, HalfWord x, HalfWord y) {
    Byte * vram = memory->memory_region(VRAM_START);

    for (int offset_y = 0; offset_y < 8; offset_y++) {
        for (int offset_x = 0; offset_x < 8; offset_x += 1) {
            Byte palette_index;

            palette_index = vram[tile_start_address + offset_x + (offset_y * 8)];

            Byte buffer_x = offset_x + x;
            Byte buffer_y = offset_y + y;

            buffer->set(buffer_x, buffer_y, get_palette_color(palette_index, palette_start_address));
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

HalfWord Display::get_palette_color(Byte index, Word palatte_start_address) {
    Byte * palette = memory->memory_region(palatte_start_address);
    return Memory::read_halfword_from_memory(palette, index*2);
}

Word Display::calculate_tile_start_address(Word charblock, Word tile) {
    return (charblock * 0x4000) + tile * 0x20;
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

Display::TiledBackground::TiledBackground(Memory * memory, Byte number) :
control(memory->memory_region(0x04000008+(2*number))),
h_scroll(memory->memory_region(0x04000010+(4*number)), 0, 15),
v_scroll(memory->memory_region(0x04000012+(4*number)), 0, 15) 
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

Display::TiledBackground::ScreenEntry::ScreenEntry(Byte * address) : 
tile_index(address, 0, 9),
horizontal_flip(address, 10),
vertical_flip(address, 11),
palette_bank(address, 12, 15)
{}

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