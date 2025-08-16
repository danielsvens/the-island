#include <raylib.h>
#include "../include/config.h"
#include "game/game.h"
#include "render/render.h"
#include "core/event_queue.h"
#include "core/input.h"

#define WIN_WIDTH   1280
#define WIN_HEIGHT  960
#define WIN_TITLE   "The Island"

#define TICK_RATE   60
#define TICK_DURATION (1.0f / TICK_RATE)

int main(void) {
  SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
  InitWindow(WIN_WIDTH, WIN_HEIGHT, WIN_TITLE);
  SetTargetFPS(60);
  DisableCursor();

  SearchAndSetResourceDir("assets");
  init_event_queue();

  Game g = {0};
  if (!game_init(&g)) return 1;

  TraceLog(LOG_INFO, "Model has %d meshes", g.floor.model.meshCount);
  TraceLog(LOG_INFO, "Model has %d materials", g.floor.model.materialCount);

  float accumulator = 0.0f;
  float last_time = GetTime();

  while (!WindowShouldClose()) {
    float current_time = GetTime();
    float frame_time = current_time - last_time;
    last_time = current_time;
    
    // Cap frame time to prevent spiral of death
    if (frame_time > 0.25f) frame_time = 0.25f;
    
    accumulator += frame_time;

    while (accumulator >= TICK_DURATION) {
      input_collect_events();
      
      game_update(&g, TICK_DURATION);
      if (g.should_quit) break;
      
      increment_tick();
      accumulator -= TICK_DURATION;
    }
    
    if (g.should_quit) break;

    BeginDrawing();
    ClearBackground(RAYWHITE);
    render_frame(&g);
    EndDrawing();
  }

  game_shutdown(&g);
  CloseWindow();
  return 0;
}

