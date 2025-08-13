#include "input.h"
#include "event_queue.h"
#include <raylib.h>

static int last_mouse_x = 0, last_mouse_y = 0;

static void push(const Event *e) { push_event(e); }

void input_collect_events(void) {
  const double t = GetTime();

  for (int k = KEY_SPACE; k <= KEY_K; ++k) {
    if (IsKeyPressed(k)) {
      Event e = {.type = EVENT_KEY_DOWN, .time = t};
      e.key.key = k;
      e.key.repeat = IsKeyPressedRepeat(k);
      e.key.alt   = IsKeyDown(KEY_LEFT_ALT)   || IsKeyDown(KEY_RIGHT_ALT);
      e.key.ctrl  = IsKeyDown(KEY_LEFT_CONTROL)|| IsKeyDown(KEY_RIGHT_CONTROL);
      e.key.shift = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
      push(&e);
    }
    if (IsKeyReleased(k)) {
      Event e = {.type = EVENT_KEY_UP, .time = t};
      e.key.key = k;
      e.key.repeat = false;
      e.key.alt   = IsKeyDown(KEY_LEFT_ALT)   || IsKeyDown(KEY_RIGHT_ALT);
      e.key.ctrl  = IsKeyDown(KEY_LEFT_CONTROL)|| IsKeyDown(KEY_RIGHT_CONTROL);
      e.key.shift = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
      push(&e);
    }
  }

  if (IsKeyPressed(KEY_ESCAPE)) {
    Event q = {.type = EVENT_QUIT, .time = t};
    push(&q);
  }

  for (int b = MOUSE_BUTTON_LEFT; b <= MOUSE_BUTTON_MIDDLE; ++b) {
    if (IsMouseButtonPressed(b)) {
      Vector2 p = GetMousePosition();
      Event e = {.type = EVENT_MOUSE_DOWN, .time = t};
      e.mouse_button.button = b; e.mouse_button.x = (int)p.x; e.mouse_button.y = (int)p.y;
      push(&e);
    }
    if (IsMouseButtonReleased(b)) {
      Vector2 p = GetMousePosition();
      Event e = {.type = EVENT_MOUSE_UP, .time = t};
      e.mouse_button.button = b; e.mouse_button.x = (int)p.x; e.mouse_button.y = (int)p.y;
      push(&e);
    }
  }

  Vector2 p = GetMousePosition();
  int x = (int)p.x, y = (int)p.y;
  int dx = x - last_mouse_x, dy = y - last_mouse_y;
  if (dx || dy) {
    Event e = {.type = EVENT_MOUSE_MOVE, .time = t};
    e.mouse_move.x = x; e.mouse_move.y = y; e.mouse_move.dx = dx; e.mouse_move.dy = dy;
    push(&e);
    last_mouse_x = x; last_mouse_y = y;
  }

  Vector2 w = GetMouseWheelMoveV();
  if (w.x != 0 || w.y != 0) {
    Event e = {.type = EVENT_MOUSE_WHEEL, .time = t};
    e.mouse_wheel.x = w.x; e.mouse_wheel.y = w.y;
    push(&e);
  }

  static int last_w = 0, last_h = 0;
  int w_now = GetScreenWidth(), h_now = GetScreenHeight();
  if (w_now != last_w || h_now != last_h) {
    if (last_w != 0) {
      Event e = {.type = EVENT_WINDOW_RESIZED, .time = t};
      e.window.width = w_now; e.window.height = h_now;
      push(&e);
    }
    last_w = w_now; last_h = h_now;
  }

  if (IsKeyPressed(KEY_SPACE)) {
    Event a = {.type = EVENT_ACTION, .time = t};
    a.action.id = ACTION_JUMP; a.action.pressed = true;
    push(&a);
  }
  if (IsKeyReleased(KEY_SPACE)) {
    Event a = {.type = EVENT_ACTION, .time = t};
    a.action.id = ACTION_JUMP; a.action.pressed = false;
    push(&a);
  }
}

