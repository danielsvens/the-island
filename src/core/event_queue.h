#pragma once
#include <stdbool.h>
#include <stdint.h>

typedef enum {
  EVENT_NONE = 0,

  // window
  EVENT_QUIT,
  EVENT_WINDOW_RESIZED,

  // Keyboard
  EVENT_KEY_DOWN,
  EVENT_KEY_UP,

  // Mouse
  EVENT_MOUSE_DOWN,
  EVENT_MOUSE_UP,
  EVENT_MOUSE_MOVE,
  EVENT_MOUSE_WHEEL,

  EVENT_ACTION,
  EVENT_AI_STATE_CHANGE,
} EventType;

typedef enum {
  ACTION_NONE = 0,
  ACTION_JUMP,
  ACTION_INTERACT,
} ActionId;

typedef struct {
  int key;          // raylib KEY_*
  bool repeat;
  bool alt, ctrl, shift;
} KeyEvent;

typedef struct {
  int button;
  int x, y;
} MouseButtonEvent;

typedef struct {
  int x, y;
  int dx, dy;
} MouseMoveEvent;

typedef struct {
  float x, y;
} MouseWheelEvent;

typedef struct {
  int width, height;
} WindowResizeEvent;

typedef struct {
  ActionId id;
  bool pressed;
} ActionEvent;

typedef struct {
  void* animal_ptr;
  int old_state;
  int new_state;
} AIStateChangeEvent;

typedef struct {
  EventType type;
  uint64_t  tick;
  union {
    KeyEvent          key;
    MouseButtonEvent  mouse_button;
    MouseMoveEvent    mouse_move;
    MouseWheelEvent   mouse_wheel;
    WindowResizeEvent window;
    ActionEvent       action;
    AIStateChangeEvent ai_state_change;
  };
} Event;

void  init_event_queue(void);
void  push_event(const Event *e);
int   poll_event(Event *out);
void  flush_events(void);
int   events_count(void);
int   events_capacity(void);

void set_current_tick(uint64_t tick);
uint64_t get_current_tick(void);
void increment_tick(void);

Event make_event(EventType type);

