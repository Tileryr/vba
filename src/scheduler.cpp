#include "src/scheduler.h"
#include "src/display.h"

// #define PROFILE

Scheduler::Scheduler(ARM7TDMI * cpu) : 
cpu(cpu), 
timers{
    Timer(&cpu->memory, &timers[1] ,0),
    Timer(&cpu->memory, &timers[2] ,1),
    Timer(&cpu->memory, &timers[3] ,2),
    Timer(&cpu->memory, nullptr ,3),
} 
{}

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
            int cpu_passed_cycles = 3;
            cpu->run_next_opcode();
            cycles_till_next_event -= cpu_passed_cycles;
            
            // for (int i = 0; i < 4; i++) {
            //     Timer timer = timers[i];
            //     if (timer.control.enabled.get() == 0 || timer.control.cascade.get() == 1) {continue;}
            //     timer.pass_cycles(cpu_passed_cycles);
            // }
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

Scheduler::Timer::Timer(Memory * memory, Timer * next_timer, Word number) : 
data(&memory->io_registers[0x00000100 + (0x04*number)], 0, 15),
control(&memory->io_registers[0x00000102 + (0x04*number)]),
next_timer(next_timer),
cycles_until_increment(get_frequency()),
reset_value(0)
{}

Scheduler::Timer::Control::Control(Byte * address) :
frequency(address, 0, 1),
cascade(address, 2),
overflow_interrupt(address, 6),
enabled(address, 7)
{}

void Scheduler::Timer::pass_cycles(Word cycles) {
    int frequency = get_frequency();
    cycles += cycles_until_increment;

    while (cycles >= 0)
    {
        if (cycles >= frequency) {
            cycles -= frequency;
            increment_data();
        } else {
            cycles_until_increment = frequency - cycles;
            cycles = -1;
        }
    }
}

void Scheduler::Timer::increment_data() {
    data.set(data.get()+1);
    if (data.get() == 0) {
        data.set(reset_value);
        if (next_timer == nullptr) return;
        if (next_timer->control.enabled.get() == 1 && next_timer->control.cascade.get() == 1) {
            next_timer->increment_data();
        }
    }
}

int Scheduler::Timer::get_frequency() {
    switch (control.frequency.get()) {
        case 0:
            return 1;
        case 1:
            return 64;
        case 2:
            return 256;
        case 3:
            return 1024;
    }

    return 0;
}