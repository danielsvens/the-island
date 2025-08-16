
#include "game.h"
#include "../core/event_queue.h"
#include "../core/input.h"
#include "assets.h"
#include "scene.h"
#include <raylib.h>

static void build_room(Scene *s, const ModelAsset *floor, const ModelAsset *wall, const ModelAsset *tree, int w, int h, Vector3 tileScale) {
  Vector3 rotAxis = {0, 1, 0};
  float defaulRotAngleDeg = 0.0f;

  float Z = (float)h / 2;
  float X = (float)w / 2.25f;

  for (int z = 0; z < h; ++z) {
    for (int x = 0; x < w; ++x) {
      Vector3 pos = { (float)x - X, 0.0f, (float)z - Z};
      scene_add(s, floor, pos, tileScale, rotAxis, 0.0f, WHITE);
    }
  }

  for (int x = 0; x < w; ++x) {
    Vector3 leftWallPos = { (float)x - X, 0.0f, (float)-Z };
    scene_add(s, wall, leftWallPos, tileScale, (Vector3){0,1,0}, 0.0f, WHITE);

    Vector3 rightWallPos = { (float)x - X, 0.0f, (float)h - Z - 0.1f };
    scene_add(s, wall, rightWallPos, tileScale, (Vector3){0,1,0}, 0.0f, WHITE);
  }

  for (int z = 0; z < h; ++z) {
    Vector3 backWallPos = { (float)-(X + 1), 0.0f, (float)z - Z };
    scene_add(s, wall, backWallPos,  tileScale, (Vector3){0,1,0}, 90.0f, WHITE);
  }

  Vector3 treePos = {-1, 0, 1};
  scene_add(s, tree, treePos, tileScale, rotAxis, defaulRotAngleDeg, WHITE);
}

bool game_init(Game *g) {
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
  if (!load_glb_model(&g->ball, "Glb/ball.glb")) return false;

  make_plane_asset(&g->floor_gen, (Color){210, 180, 140, 255});
  make_wall_asset(&g->wall_gen, 0.1f, 2.5f, (Color){222, 184, 135, 255});

  scene_reset(&g->scene);
  world_init(&g->world);

  Vector3 tileScale = (Vector3){1.0f, 1.0f, 1.0f};
  build_room(&g->scene, &g->floor, &g->wall, &g->tree, 18, 18, tileScale);

  Vector3 ball_pos = {5, 1, 1};
  create_ball(&g->world, ball_pos, &g->ball,  0.5f, 1.0f, 0.7f, 0.95f);
 
  // lighting
  const float ambient[4] = {0.15f, 0.15f, 0.18f, 1.0f};

  if (!lighting_init(&g->lights, ambient, (Vector3){-0.4f,-1.0f,-0.3f}, WHITE)) {
    return false;
  }

  lighting_apply_to_model(&g->lights, &g->floor.model, (Color){210,180,140,255});
  lighting_apply_to_model(&g->lights, &g->wall.model, (Color){222,184,135,255});
  lighting_apply_to_model(&g->lights, &g->tree.model, (Color){222,184,135,255});
  lighting_apply_to_model(&g->lights, &g->ball.model, (Color){150,175,220,255});

  return true;
}

void game_update(Game *g, float dt) {
  (void)dt;

  Event e;
  while (poll_event(&e)) {
    switch (e.type) {
      case EVENT_QUIT:
        g->should_quit = true;
        break;

      case EVENT_KEY_DOWN:
        if (e.key.key == KEY_SPACE && !e.key.repeat) {
          TraceLog(LOG_INFO, "SPACE pressed at tick %llu (alt=%d ctrl=%d shift=%d)",
                   e.tick, e.key.alt, e.key.ctrl, e.key.shift);
        }
        break;

      case EVENT_MOUSE_MOVE:
        break;

      case EVENT_WINDOW_RESIZED:
        TraceLog(LOG_INFO, "Resized to %dx%d at tick %llu", 
                 e.window.width, e.window.height, e.tick);
        break;

      case EVENT_ACTION:
        if (e.action.id == ACTION_JUMP && e.action.pressed) {
          TraceLog(LOG_INFO, "Action: Fly up!! at tick %llu", e.tick);
        }
        break;

      default: break;
    }
  }

  UpdateCamera(&g->camera, CAMERA_FREE);
  lighting_update(&g->lights, &g->camera);
  update_physics(&g->world, dt);
}
void game_shutdown(Game *g) {
  unload_model_asset(&g->floor);
  unload_model_asset(&g->wall);
  unload_model_asset(&g->tree);
  unload_model_asset(&g->ball);
  unload_atlas(g->atlas);
  lighting_shutdown(&g->lights);
}

