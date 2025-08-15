#pragma once
#include "assets.h"
#include "scene.h"
#include <raylib.h>
#include <stdbool.h>
#include "lighting.h"

typedef struct {
  Texture2D atlas;

  ModelAsset floor;
  ModelAsset wall;
  ModelAsset tree;

  ModelAsset floorGen;
  ModelAsset wallGen;

  Lighting lights;

  Scene scene;

  Camera camera;

  bool should_quit;
} Game;

bool game_init(Game* g);
void game_update(Game* g, float dt);
void game_shutdown(Game* g);

