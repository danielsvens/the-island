#include "event_queue.h"
#include <stdio.h>

#ifndef EVENT_QUEUE_CAP
#define EVENT_QUEUE_CAP 256
#endif
#define MASK (EVENT_QUEUE_CAP - 1)
_Static_assert((EVENT_QUEUE_CAP & MASK) == 0, "EVENT_QUEUE_CAP must be power of two");

static Event buf[EVENT_QUEUE_CAP];
static int head = 0, tail = 0;
static int dropped = 0;
static uint64_t current_tick = 0;

void init_event_queue(void) { 
  head = tail = dropped = current_tick = 0;
}

static int size(void) { return head - tail; }

void push_event(const Event *e) {
  if (size() >= EVENT_QUEUE_CAP) {
    tail++;
    if (dropped++ < 1) fprintf(stderr, "[event] queue overflow, dropping oldest\n");
  }
  buf[head & MASK] = *e;
  head++;
}

int poll_event(Event *out) {
  if (head == tail) return 0;
  *out = buf[tail & MASK];
  tail++;
  return 1;
}

void flush_events(void) { tail = head; }
int  events_count(void) { return size(); }
int  events_capacity(void) { return EVENT_QUEUE_CAP; }

// Tick management functions
void set_current_tick(uint64_t tick) {
  current_tick = tick;
}

uint64_t get_current_tick(void) {
  return current_tick;
}

void increment_tick(void) {
  current_tick++;
}

// Convenience function to create events with current tick
Event make_event(EventType type) {
  Event e = {0};
  e.type = type;
  e.tick = current_tick;
  return e;
}

