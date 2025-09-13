#include "src/scheduler.h"
#include "src/display.h"

// #define PROFILE

Scheduler::Scheduler(ARM7TDMI * cpu) : cpu(cpu) {}

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
    passed_nanoseconds = SDL_GetTicksNS() - total_passed_nanoseconds;
    passed_milliseconds = SDL_GetTicks() - total_passed_milliseconds;

    int cycles_to_pass = passed_milliseconds * CYCLES_PER_MILISECOND;
    int event_total = 0;

    int time = SDL_GetTicks();
    u_int64_t time_ns = SDL_GetTicksNS();

    int passed_cycles = cycles_to_pass;

    while (passed_cycles > 0) {
        ScheduledEvent next_event = events.front();
        int cycles_till_next_event = next_event.cycles_till_done;

        while (cycles_till_next_event > 0) {
            cpu->run_next_opcode();
            cycles_till_next_event -= 3;
        }
        
        move_events_forward(next_event.cycles_till_done);
        passed_cycles -= next_event.cycles_till_done;

        next_event.event();

        events.pop_front();
        event_total++;
    }

    #ifdef PROFILE
        int passed_time = SDL_GetTicks()-time;
        u_int64_t passed_time_ns = SDL_GetTicksNS()-time_ns;
        int cycles_per_second = 0;
        if (passed_time > 0) {
            cycles_per_second = (cycles_to_pass/passed_time)*1000;
        }
        SDL_Log("passed_cycles: %d, time taken (ms): %d, time taken (ns): %lu, cycles per second: %d", cycles_to_pass, passed_time, passed_time_ns, cycles_per_second);
    #endif
    
    total_passed_milliseconds = SDL_GetTicks();
    passed_nanoseconds = SDL_GetTicksNS();
}    