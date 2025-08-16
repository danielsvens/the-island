// Example demonstrating tick-based event scheduling
#include "event_queue.h"
#include <stdio.h>

// Example: Schedule an event to happen in the future
void schedule_delayed_action(ActionId action, uint64_t delay_ticks) {
    Event e = make_event(EVENT_ACTION);
    e.action.id = action;
    e.action.pressed = true;
    e.tick = get_current_tick() + delay_ticks;
    
    // In a real implementation, you'd add this to a separate scheduled events queue
    // For demonstration purposes, we're showing the concept
    printf("Scheduling action %d to happen at tick %llu (current: %llu)\n", 
           action, e.tick, get_current_tick());
}

// Example: Process scheduled events
void process_scheduled_events(void) {
    uint64_t current_tick = get_current_tick();
    
    // In a real implementation, you'd check your scheduled events queue
    // and push any events whose time has come
    
    // This is just a conceptual example
    static uint64_t last_scheduled_event = 0;
    if (current_tick > 0 && current_tick % 180 == 0 && current_tick != last_scheduled_event) {
        Event e = make_event(EVENT_ACTION);
        e.action.id = ACTION_INTERACT;
        e.action.pressed = true;
        push_event(&e);
        last_scheduled_event = current_tick;
        printf("Triggered scheduled event at tick %llu\n", current_tick);
    }
}

// Example: Deterministic input replay
typedef struct {
    Event event;
    uint64_t recorded_tick;
} RecordedEvent;

static RecordedEvent replay_buffer[1000];
static int replay_count = 0;
static int replay_index = 0;
static bool replaying = false;

void start_recording(void) {
    replay_count = 0;
    printf("Started recording events at tick %llu\n", get_current_tick());
}

void record_event(const Event *e) {
    if (replay_count < 1000) {
        replay_buffer[replay_count].event = *e;
        replay_buffer[replay_count].recorded_tick = e->tick;
        replay_count++;
    }
}

void start_replay(void) {
    replay_index = 0;
    replaying = true;
    printf("Started replaying %d events from tick %llu\n", replay_count, get_current_tick());
}

void replay_events(void) {
    if (!replaying || replay_index >= replay_count) return;
    
    uint64_t current_tick = get_current_tick();
    uint64_t replay_start_tick = current_tick - replay_buffer[0].recorded_tick;
    
    while (replay_index < replay_count) {
        RecordedEvent *re = &replay_buffer[replay_index];
        uint64_t target_tick = replay_start_tick + re->recorded_tick;
        
        if (current_tick >= target_tick) {
            Event e = re->event;
            e.tick = current_tick; // Update tick to current time
            push_event(&e);
            replay_index++;
            printf("Replayed event at tick %llu (original: %llu)\n", 
                   current_tick, re->recorded_tick);
        } else {
            break;
        }
    }
    
    if (replay_index >= replay_count) {
        replaying = false;
        printf("Replay finished at tick %llu\n", current_tick);
    }
}
