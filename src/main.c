#include <raylib.h>
#include "../include/config.h"
#include "game/game.h"
#include "render/render.h"
#include "core/event_queue.h"

#define WIN_WIDTH   800
#define WIN_HEIGHT  600
#define WIN_TITLE   "The Island"

int main(void) {
  SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
  InitWindow(WIN_WIDTH, WIN_HEIGHT, WIN_TITLE);
  SetTargetFPS(60);
  DisableCursor();

  SearchAndSetResourceDir("assets");
  init_event_queue();

  Game g = {0};
  if (!game_init(&g)) return 1;

  TraceLog(LOG_INFO, "Model has %d meshes", g.model.meshCount);
  TraceLog(LOG_INFO, "Model has %d materials", g.model.materialCount);

  while (!WindowShouldClose()) {
    float dt = GetFrameTime();
    game_update(&g, dt);
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

