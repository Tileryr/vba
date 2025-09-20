#define SDL_MAIN_USE_CALLBACKS

#include <map>

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

#include "src/cpu/opcodes/arm/multiply.h"

#define SCALE 3

static ARM7TDMI * cpu = new ARM7TDMI();
static Scheduler * scheduler = new Scheduler(cpu);
static Display * display = nullptr;

static SDL_Window * window = nullptr;
static SDL_Renderer * renderer = nullptr;

Word ticks_since_last_render = 0;

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

    display->start_draw_loop(scheduler);

    scheduler->total_passed_milliseconds = SDL_GetTicks();
    scheduler->total_passed_nanoseconds = SDL_GetTicksNS();

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    static std::map<SDL_Scancode, u_int16_t> input_bit_map = {
        {SDL_SCANCODE_Z, 0}, // A
        {SDL_SCANCODE_X, 1}, // B
        {SDL_SCANCODE_C, 2}, // SELECT
        {SDL_SCANCODE_V, 3}, // START
        {SDL_SCANCODE_RIGHT, 4}, // RIGHT
        {SDL_SCANCODE_LEFT, 5}, // LEFT
        {SDL_SCANCODE_UP, 6}, // UP
        {SDL_SCANCODE_DOWN, 7}, // DOWN
        {SDL_SCANCODE_A, 8}, // R
        {SDL_SCANCODE_D, 9}, // L
    };

    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }

    if (event->type != SDL_EVENT_KEY_DOWN && event->type != SDL_EVENT_KEY_UP) {
        return SDL_APP_CONTINUE;
    }

    auto iterator = input_bit_map.find(event->key.scancode);
    if (iterator != input_bit_map.end()) {
        HalfWord target_bit = iterator->second;
        
        HalfWord current_input = cpu->read_halfword_from_memory(KEY_INPUT_ADDRESS);
        bool released = event->type == SDL_EVENT_KEY_UP;
        Utils::write_bit(&current_input, target_bit, released);
        cpu->write_halfword_to_memory(KEY_INPUT_ADDRESS, current_input);
    }
    
    return SDL_APP_CONTINUE;
}   

int current_scanline = 0;

SDL_AppResult SDL_AppIterate(void *appstate) {   
    scheduler->tick();

    ticks_since_last_render += scheduler->passed_milliseconds;
    if (ticks_since_last_render > 1000/60) {
        current_scanline %= SCREEN_HEIGHT;
        if (current_scanline == 0) {
            memset(display->screen_buffers, 0xFF, sizeof(display->screen_buffers));
        }
        display->update_scanline(current_scanline++);

        display->render();
        ticks_since_last_render = 0;
    }
    
    
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    delete scheduler;
    delete display;
    delete cpu;
}