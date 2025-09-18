#ifndef SCHEDULER_INCLUDED
#define SCHEDULER_INCLUDED

#include <SDL3/SDL.h>
#include <functional>
#include <list>
#include <map>

#include "src/cpu/cpu.h"
#include "src/cpu/cpu_types.h"
#include "src/cpu/bit_region.h"

#define CYCLES_PER_SECOND 16777216
#define CYCLES_PER_MILISECOND 16777

typedef struct Scheduler {
    Scheduler(ARM7TDMI * cpu);

    typedef struct ScheduledEvent {
        Word cycles_till_done;
        std::function<void()> event;
    } ScheduledEvent;

    typedef struct Timer {
        Timer(Memory * memory, Timer * timer, Word number);
        struct Control {
            Control(Byte * address);
            BitRegion frequency;
            BitRegion cascade;
            BitRegion overflow_interrupt;
            BitRegion enabled;
        } control;

        Timer * next_timer;
        BitRegion data;

        int cycles_until_increment;
        HalfWord reset_value;

        void increment_data();
        void pass_cycles(Word cycles);
        int get_frequency();
    } Timer;

    Timer timers[4];

    ARM7TDMI * cpu;
    Word total_passed_milliseconds;
    u_int64_t total_passed_nanoseconds;

    Word passed_milliseconds;
    Word passed_nanoseconds;

    std::list<ScheduledEvent> events;

    void schedule_event(Word cycles, std::function<void()> event);
    void move_events_forward(Word cycles);
    void tick();  
} Scheduler;

#endif