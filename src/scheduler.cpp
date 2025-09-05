#include "src/scheduler.h"
#include "src/display.h"

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
    int passed_cycles = (SDL_GetTicks() - total_passed_milliseconds) * CYCLES_PER_MILISECOND;

    while (passed_cycles > 0) {
        ScheduledEvent next_event = events.front();

        move_events_forward(next_event.cycles_till_done);
        passed_cycles -= next_event.cycles_till_done;

        next_event.event();

        events.pop_front();
    }
    
    total_passed_milliseconds = SDL_GetTicks();
}    