#pragma once
#include "../game/game.h"
#include <raylib.h>

BoundingBox get_transformed_bbox(Model model, Vector3 pos, Vector3 scale);
void render_frame(const Game *g);

