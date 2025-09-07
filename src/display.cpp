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

void Display::update_sprites() {
    for (int i = 0; i < 128; i++) {
        render_sprite(i);
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

    SDL_Log("sprite: %d, value: %04x", sprite_number, attribute_0_value);

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

    HalfWord tile_size_x = pixel_size_x / 8;
    HalfWord tile_size_y = pixel_size_y / 8;

    for (int y = 0; y < tile_size_y; y++) {
        for (int x = 0; x < tile_size_x; x++) {
            bool one_dimensional = display_control.vram_mapping.get() == 1;
            if (one_dimensional) {
                Word tile_x = attribute_1.x + x*8;
                Word tile_y = attribute_0.y + y*8;

                SDL_Log("%d", (y*tile_size_x + x));
                render_tile_4bpp(
                    4, 
                    attribute_2.tile_index + (y*tile_size_x + x), 
                    attribute_2.palette_bank,
                    memory->memory_region(OBJ_PALETTE_RAM_START),
                    tile_x,
                    tile_y
                );
            } else {

            }
        }
    }
}

void Display::render_tile_4bpp(Byte charblock, HalfWord tile, Byte palbank, Byte * palette_memory, HalfWord x, HalfWord y) {
    Byte * vram = memory->memory_region(VRAM_START);

    for (int offset_y = 0; offset_y < 8; offset_y++) {
        for (int offset_x = 0; offset_x < 4; offset_x += 1) {
            Word tile_start_address = (charblock * 0x4000) + tile * 0x20;

            Byte palette_index_low;
            Byte palette_index_high;

            palette_index_low = vram[tile_start_address + offset_x + (offset_y * 4)] & 0xF;
            palette_index_high = (vram[tile_start_address + offset_x + (offset_y * 4)] >> 4) & 0xF;

            palette_index_low |= (palbank << 4);
            palette_index_high |= (palbank << 4);

            Byte screen_x = offset_x*2 + x;
            Byte screen_y = offset_y + y;

            screen[screen_x][screen_y] = Memory::read_halfword_from_memory(palette_memory, palette_index_low*2);
            screen[screen_x + 1][screen_y] = Memory::read_halfword_from_memory(palette_memory, palette_index_high*2);
        }
    }

    for (int i = 0; i < 16; i++) {
        screen[(SCREEN_WIDTH - i) - 1][SCREEN_HEIGHT - 1] = Memory::read_halfword_from_memory(palette_memory, (palbank << 4) | i); 
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