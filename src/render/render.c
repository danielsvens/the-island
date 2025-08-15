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

void render_frame(const Game* g) {
  BeginMode3D(g->camera);
  DrawGrid(50, 1.0f);

  for (int i = 0; i < g->scene.count; ++i) {
    const Instance *it = &g->scene.inst[i];
    DrawModelEx(it->asset->model, it->pos, it->rotAxis, it->rotAngleDeg, it->scale, it->tint);
    //DrawModelWiresEx(it->asset->model, it->pos, it->rotAxis, it->rotAngleDeg, it->scale, BLACK);

    // draw grid
    //DrawModelEx(g->buildingBox, g->position, g->position, 0.0f, g->scale, WHITE);
    //DrawBoundingBox(get_transformed_bbox(g->model, g->position, g->scale), RED);
  }

  EndMode3D();
  DrawText("Press SPACE to trigger event", 10, 10, 20, DARKGRAY);
}

