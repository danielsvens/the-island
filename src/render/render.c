#include "render.h"
#include "../game/scene.h"
#include <raylib.h>

void render_frame(const Game *g) {
  BeginMode3D(g->camera);
  DrawGrid(50, 1.0f);

  render_scene(&g->scene);
  render_objects(&g->world);

  EndMode3D();
  DrawText("Press SPACE to trigger event", 10, 10, 20, DARKGRAY);
  DrawFPS(10, 40);
}

