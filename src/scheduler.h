#ifndef SCHEDULER_INCLUDED
#define SCHEDULER_INCLUDED

#include <SDL3/SDL.h>

#define CYCLES_PER_SECOND 16777216
#define CYCLES_PER_MILISECOND 200

typedef struct Scheduler {
    int total_passed_cycles;

    void tick() {
        int passed_cycles = SDL_GetTicks() * CYCLES_PER_MILISECOND;

        while (passed_cycles != 0) {

            passed_cycles -= 1;
            total_passed_cycles += 1;
        }
    }
} Scheduler;

#endif