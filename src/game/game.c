
#include "game.h"
#include "../core/event_queue.h"
#include "../core/input.h"
#include <raylib.h>

bool game_init(Game* g) {
  g->scale    = (Vector3){0.1f, 0.1f, 0.1f};
  g->position = (Vector3){5.0f, 0.0f, -5.0f};
  g->should_quit = false;

  g->model = LoadModel("OBJ/SM_Buildings_Floor_1x1_01.obj");
  g->texture = LoadTexture("Textures/PolygonPrototype_Texture_01.png");
  g->model.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = g->texture;

  g->buildingBox = LoadModel("OBJ/SM_Buildings_Block_1x1_01.obj");
  g->buildingBox.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = g->texture;

  g->camera.position = (Vector3){ 10.0f, 15.0f, 10.0f };
  g->camera.target   = (Vector3){  0.0f,  0.0f,  0.0f };
  g->camera.up       = (Vector3){  0.0f,  1.0f,  0.0f };
  g->camera.fovy = 45.0f;
  g->camera.projection = CAMERA_PERSPECTIVE;
  return true;
}

void game_update(Game* g, float dt) {
  (void)dt;

  input_collect_events();

  Event e;
  while (poll_event(&e)) {
    switch (e.type) {
      case EVENT_QUIT:
        g->should_quit = true;
        break;

      case EVENT_KEY_DOWN:
        if (e.key.key == KEY_SPACE && !e.key.repeat) {
          TraceLog(LOG_INFO, "SPACE pressed (alt=%d ctrl=%d shift=%d)",
                   e.key.alt, e.key.ctrl, e.key.shift);
        }
        break;

      case EVENT_MOUSE_MOVE:
        break;

      case EVENT_WINDOW_RESIZED:
        TraceLog(LOG_INFO, "Resized to %dx%d", e.window.width, e.window.height);
        break;

      case EVENT_ACTION:
        if (e.action.id == ACTION_JUMP && e.action.pressed) {
          TraceLog(LOG_INFO, "Action: JUMP!");
        }
        break;

      default: break;
    }
  }

  UpdateCamera(&g->camera, CAMERA_FREE);
}

void game_shutdown(Game* g) {
  UnloadTexture(g->texture);
  UnloadModel(g->model);
  UnloadModel(g->buildingBox);
}

