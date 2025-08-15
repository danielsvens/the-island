#pragma once
#include <raylib.h>

typedef struct {
    Model model;
} ModelAsset;

Texture2D load_atlas(const char *path);
void unload_atlas(Texture2D tex);

bool load_model_with_atlas(ModelAsset *out, const char *objPath, Texture2D atlas);
bool load_glb_model(ModelAsset *out, const char *objPath);

bool make_plane_asset(ModelAsset* out, Color color);
bool make_wall_asset(ModelAsset* out, float thickness, float height, Color color);

void unload_model_asset(ModelAsset *m);

