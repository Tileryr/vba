#ifndef SCHEDULER_INCLUDED
#define SCHEDULER_INCLUDED

#include <SDL3/SDL.h>
#include <functional>
#include <map>

#define CYCLES_PER_SECOND 16777216
#define CYCLES_PER_MILISECOND 1677

typedef struct Scheduler {
    u_int64_t total_passed_cycles;
    int total_passed_milliseconds;
    
    std::map<u_int64_t, std::function<void()>> events;

    Scheduler();

    void initialize();

    void update_time();

    void schedule_event(int cycles, std::function<void()> event);

    void tick();  
} Scheduler;

#endif