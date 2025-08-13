#pragma once
#include <raylib.h>
#include <stdbool.h>

typedef struct {
  Vector3 pos;
  Vector3 vel;

  float max_speed;
  float radius;
} Animal;
