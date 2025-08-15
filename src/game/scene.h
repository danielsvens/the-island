#pragma once
#include <raylib.h>
#include <stddef.h>
#include "assets.h"

#define MAX_INSTANCES 1024

typedef struct {
    const ModelAsset *asset;
    Vector3 pos;
    Vector3 scale;
    Vector3 rotAxis;
    float rotAngleDeg;
    Color tint;
} Instance;

typedef struct {
    Instance inst[MAX_INSTANCES];
    int count;
} Scene;

void scene_reset(Scene *s);
int scene_add(Scene *s, const ModelAsset *asset, Vector3 pos, Vector3 scale, Vector3 rotAxis, float rotAngleDeg, Color tint);

