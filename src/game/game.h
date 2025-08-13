#pragma once
#include <raylib.h>
#include <stdbool.h>

typedef struct {
  Model   model, buildingBox;
  Texture texture;

  Vector3 scale;
  Vector3 position;

  Camera  camera;

  bool    should_quit;
} Game;

bool game_init(Game* g);
void game_update(Game* g, float dt);
void game_shutdown(Game* g);

