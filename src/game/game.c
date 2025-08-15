
#include "game.h"
#include "../core/event_queue.h"
#include "../core/input.h"
#include "assets.h"
#include <raylib.h>

static void build_room(Scene *s, const ModelAsset *floor, const ModelAsset *wall,
                       int w, int h, Vector3 tileScale)
{
    for (int z = 0; z < h; ++z) {
        for (int x = 0; x < w; ++x) {
            Vector3 pos = { (float)x - 2, 0.0f, (float)z - 3 };
            scene_add(s, floor, pos, tileScale, (Vector3){0,1,0}, 0.0f, WHITE);
        }
    }
   
    for (int x = 0; x < w; ++x) {
        Vector3 left_wall_pos = { (float)x - 2, 0.0f, (float)-3 };
        scene_add(s, wall, left_wall_pos, tileScale, (Vector3){0,1,0}, 0.0f, WHITE);

        Vector3 right_wall_pos = { (float)x - 2, 0.0f, (float)h - 3.1f };
        scene_add(s, wall, right_wall_pos, tileScale, (Vector3){0,1,0}, 0.0f, WHITE);
    }


    for (int z = 0; z < h; ++z) {
        Vector3 back_wall_pos = { (float)-3, 0.0f, (float)z - 3.0f };
        scene_add(s, wall, back_wall_pos,  tileScale, (Vector3){0,1,0}, 90.0f, WHITE);
    }
}

bool game_init(Game* g) {
  g->should_quit = false;

  g->camera.position = (Vector3){ 10.0f, 15.0f, 10.0f };
  g->camera.target = (Vector3){  0.0f,  0.0f,  0.0f };
  g->camera.up = (Vector3){  0.0f,  1.0f,  0.0f };
  g->camera.fovy = 45.0f;
  g->camera.projection = CAMERA_PERSPECTIVE;

  g->atlas = load_atlas("Textures/PolygonPrototype_Texture_01.png");

  //if (!load_model_with_atlas(&g->floor, "Glb/floor.glb", g->atlas)) return false;
  if (!load_glb_model(&g->floor, "Glb/floor.glb")) return false;
  if (!load_glb_model(&g->wall, "Glb/wall.glb")) return false;
  if (!load_glb_model(&g->tree, "Glb/tree.glb")) return false;

  make_plane_asset(&g->floorGen, (Color){210, 180, 140, 255});
  make_wall_asset(&g->wallGen, 0.1f, 2.5f, (Color){222, 184, 135, 255});

  scene_reset(&g->scene);
  Vector3 tileScale = (Vector3){1.0f, 1.0f, 1.0f};
  build_room(&g->scene, &g->floor, &g->wall, 6, 6, tileScale);


  // Lightning
  g->lighting = LoadShader("shaders/lighting.vs", "shaders/lighting.fs");

  g->locViewPos = GetShaderLocation(g->lighting, "viewPos");
  g->locAmbient = GetShaderLocation(g->lighting, "ambient");

  float ambient[4] = { 0.15f, 0.15f, 0.18f, 1.0f };
  SetShaderValue(g->lighting, g->locAmbient, ambient, SHADER_UNIFORM_VEC4);

  for (int i = 0; i < g->floor.model.materialCount; ++i)
      g->floor.model.materials[i].shader = g->lighting;
  for (int i = 0; i < g->wall.model.materialCount; ++i)
      g->wall.model.materials[i].shader = g->lighting;

  g->light0 = CreateLight(LIGHT_DIRECTIONAL,
                          (Vector3){ 0.0f, 0.0f, 0.0f },   // position (unused for directional)
                          (Vector3){ -0.4f, -1.0f, -0.3f },// direction (x,y,z)
                          WHITE,                            // color
                          g->lighting);                     // shader to update


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
  SetShaderValue(g->lighting, g->locViewPos, &g->camera.position, SHADER_UNIFORM_VEC3);

  UpdateLightValues(g->lighting, g->light0);
}
void game_shutdown(Game* g) {
  unload_model_asset(&g->floor);
  unload_model_asset(&g->wall);
  unload_model_asset(&g->tree);
  unload_atlas(g->atlas);
  UnloadShader(g->lighting);
}

