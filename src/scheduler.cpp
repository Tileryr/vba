#include "src/scheduler.h"
#include "src/display.h"

#define PROFILE

void Scheduler::schedule_event(Word cycles, std::function<void()> event) {
    ScheduledEvent scheduled_event = {cycles, event};
    events.push_back(scheduled_event);
    events.sort([](const ScheduledEvent& first, const ScheduledEvent& second){
        return first.cycles_till_done < second.cycles_till_done;
    });
}
 
void Scheduler::move_events_forward(Word cycles) {
    for (auto & scheduled_event : events) {
        scheduled_event.cycles_till_done -= cycles;
    }
}

void Scheduler::tick() {
    int cycles_to_pass = (SDL_GetTicks() - total_passed_milliseconds) * CYCLES_PER_MILISECOND*4;
    int event_total = 0;

    int time = SDL_GetTicks();
    int passed_cycles = cycles_to_pass;

    while (passed_cycles > 0) {
        ScheduledEvent next_event = events.front();

        move_events_forward(next_event.cycles_till_done);
        passed_cycles -= next_event.cycles_till_done;

        next_event.event();

        events.pop_front();
        event_total++;
    }

    #ifdef PROFILE
        int passed_time = SDL_GetTicks()-time;
        int cycles_per_second = 0;
        if (passed_time > 0) {
            cycles_per_second = (cycles_to_pass/passed_time)*1000;
        }
        SDL_Log("passed_cycles: %d, time taken (ms): %d, cycles per second: %d", cycles_to_pass, passed_time, cycles_per_second);
    #endif
    
    total_passed_milliseconds = SDL_GetTicks();
}    