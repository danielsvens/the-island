// render/render.c
#include "render.h"
#include "../game/scene.h"
#include <raylib.h>

BoundingBox get_transformed_bbox(Model model, Vector3 pos, Vector3 scale) {
  BoundingBox box = GetModelBoundingBox(model);
  box.min.x = pos.x + box.min.x * scale.x;
  box.min.y = pos.y + box.min.y * scale.y;
  box.min.z = pos.z + box.min.z * scale.z;
  box.max.x = pos.x + box.max.x * scale.x;
  box.max.y = pos.y + box.max.y * scale.y;
  box.max.z = pos.z + box.max.z * scale.z;
  return box;
}

void render_frame(const Game *g) {
  BeginMode3D(g->camera);
  DrawGrid(50, 1.0f);

  render_scene(&g->scene);
  render_objects(&g->world);

  EndMode3D();
  DrawText("Press SPACE to trigger event", 10, 10, 20, DARKGRAY);
  DrawFPS(10, 40);
}

