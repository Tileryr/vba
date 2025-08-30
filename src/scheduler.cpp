#include "src/scheduler.h"
#include "src/display.h"

Scheduler::Scheduler() {
    update_time();
    initialize();
}

void Scheduler::initialize() {}

void Scheduler::update_time() {
    total_passed_milliseconds = SDL_GetTicks();
    total_passed_cycles = total_passed_milliseconds * CYCLES_PER_MILISECOND;
}

void Scheduler::schedule_event(int cycles, std::function<void()> event) {
    u_int64_t event_time = cycles + total_passed_cycles;
    bool event_already_exists = events.count(cycles);
    if (event_already_exists) {
        std::function<void()> current_event = events.find(cycles)->second;
        events.insert_or_assign(event_time, [event, current_event](){
            current_event();
            event();
        });
    } else {

        events.insert_or_assign(event_time, event);
    }
}

void Scheduler::tick() {
    update_time();

    for (auto iterator = events.begin(); iterator != events.upper_bound(total_passed_cycles); iterator++) {
        iterator->second();
    }

    auto iterator_start = events.begin();
    auto iterator_end = events.upper_bound(total_passed_cycles);
    events.erase(iterator_start, iterator_end);
}    