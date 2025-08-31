#define SDL_MAIN_USE_CALLBACKS

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_render.h>

#include "./cpu/cpu.h"
#include "./utils.h"
#include "./cpu/alu.h"
#include "src/cpu/opcodes/arm/data_processing.h"
#include "src/display.h"
#include "src/scheduler.h"

#include <stdlib.h>
#include <cstdio>

#define SCALE 3

static ARM7TDMI * cpu = new ARM7TDMI();
static Scheduler * scheduler = new Scheduler();

static Display * display = nullptr;
static SDL_Window * window = nullptr;
static SDL_Renderer * renderer = nullptr;

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv) {
    // SDL_SetAppMetadata();
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL initialization failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    window = SDL_CreateWindow("VBA", 240 * SCALE, 160 * SCALE, SDL_WINDOW_RESIZABLE);
    if (window == nullptr) {
        SDL_Log("SDL window creation failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    
    renderer = SDL_CreateRenderer(window, NULL);
    if (renderer == nullptr) {
        SDL_Log("SDL renderer creation failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_SetRenderScale(renderer, SCALE, SCALE);

    display = new Display(renderer, &cpu->memory);
    display->start_draw_loop(scheduler);

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

        fread(&cpu->memory.memory[GAMEPAK_ROM_START], 1, file_length, fileptr);
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

SDL_AppResult SDL_AppIterate(void *appstate) {
    // SDL_Log("%d", SDL_GetTicks());
    if (cpu->read_register(REGISTER_PC) > 0xFFFFFFF) {
        return SDL_APP_SUCCESS;
    } else {
        for (int i = 0; i < 4; i++) {
            cpu->run_next_opcode();
        }
    }
    
    scheduler->tick();
    display->update_screen_bgmode_4();
    display->render();
    

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    delete scheduler;
    delete display;
    delete cpu;
}