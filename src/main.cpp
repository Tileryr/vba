#define SDL_MAIN_USE_CALLBACKS

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_render.h>

#include "./cpu/cpu.h"
#include "./utils.h"
#include "./cpu/alu.h"
#include "src/cpu/opcodes/arm/data_processing.h"

#include <stdlib.h>
#include <cstdio>

static ARM7TDMI * cpu = new ARM7TDMI();
static SDL_Window * window = nullptr;
static SDL_Renderer * renderer = nullptr;

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv) {
    // SDL_SetAppMetadata();
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL initialization failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    window = SDL_CreateWindow("VBA", 500, 500, SDL_WINDOW_RESIZABLE);
    if (window == nullptr) {
        SDL_Log("SDL window creation failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    
    renderer = SDL_CreateRenderer(window, NULL);
    if (renderer == nullptr) {
        SDL_Log("SDL renderer creation failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    cpu->memory = (Byte *)malloc(0xFFFFFFF);
    cpu->write_halfword_to_memory(0x04000004, 1);

    cpu->skip_bios();
    if (argc > 1) {
        FILE * fileptr;
        
        std::string filename = "test-roms/";
        filename = filename + argv[1];

        SDL_Log("%s", filename.c_str());

        long file_length;

        fileptr = fopen(filename.c_str(), "rb");
        fseek(fileptr, 0, SEEK_END);
        file_length = ftell(fileptr);
        rewind(fileptr);

        fread(&cpu->memory[GAMEPAK_ROM_START], 1, file_length, fileptr);
        fclose(fileptr);
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }
    return SDL_APP_CONTINUE;
}   

static void render() {
    Word vram_address_start = 0x06000000;

    // MODE 4
    Byte * vram = cpu->memory_region(VRAM_START);
    Byte * background_palette = cpu->memory_region(BG_PALETTE_RAM_START);

    SDL_SetRenderDrawColor(renderer, 50, 50, 50, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);

    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            Byte palette_color = background_palette[(y*SCREEN_WIDTH) + x];

            Byte r = Utils::read_bit_range(palette_color, 0,  4);
            Byte g = Utils::read_bit_range(palette_color, 5,  9);
            Byte b = Utils::read_bit_range(palette_color, 10, 14);

            SDL_SetRenderDrawColor(renderer, r << 3, g << 3, b << 3, SDL_ALPHA_OPAQUE);
            SDL_RenderPoint(renderer, x, y);
        }
    }
    
    SDL_SetRenderDrawColor(renderer, 25 << 3, 25 << 3, 25 << 3, SDL_ALPHA_OPAQUE);
    SDL_RenderPoint(renderer, 10, 10);
    // MODE 3
    // for (int y = 0; y < 160; y++) {
    //     for (int x = 0; x < 240; x++) {
    //         HalfWord color = cpu->read_halfword_from_memory(vram_address_start + (x*2) * y);

    //         Byte r = Utils::read_bit_range(color, 0,  4);
    //         Byte g = Utils::read_bit_range(color, 5,  9);
    //         Byte b = Utils::read_bit_range(color, 10, 14);

    //         SDL_SetRenderDrawColor(renderer, r, g, b, SDL_ALPHA_OPAQUE);
    //         SDL_RenderPoint(renderer, x, y);
    //     }
    // }
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    // SDL_Log("%d", SDL_GetTicks());
    if (cpu->read_register(REGISTER_PC) < 0xFFFFFFF) {
        cpu->run_next_opcode();
    } else {
        return SDL_APP_SUCCESS;
    }
    
    

    render();
    SDL_RenderPresent(renderer);

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    delete cpu;
}