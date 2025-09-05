#ifndef SCHEDULER_INCLUDED
#define SCHEDULER_INCLUDED

#include <SDL3/SDL.h>
#include <functional>
#include <list>
#include <map>

#include "src/cpu/cpu_types.h"

#define CYCLES_PER_SECOND 16777216
#define CYCLES_PER_MILISECOND 1677

typedef struct Scheduler {
    typedef struct ScheduledEvent {
        Word cycles_till_done;
        std::function<void()> event;
    } ScheduledEvent;

    Word total_passed_milliseconds;
    
    std::list<ScheduledEvent> events;

    void schedule_event(Word cycles, std::function<void()> event);

    void move_events_forward(Word cycles);

    void tick();  
} Scheduler;

#endif