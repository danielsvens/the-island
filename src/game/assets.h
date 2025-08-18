#pragma once
#include <raylib.h>

typedef struct {
  Model model;
  BoundingBox local_bbox;  // Cached bounding box in model space
} ModelAsset;

Texture2D load_atlas(const char *path);
void unload_atlas(Texture2D tex);

bool load_model_with_atlas(ModelAsset *out, const char *obj_path, Texture2D atlas);
bool load_glb_model(ModelAsset *out, const char *obj_path);

bool make_plane_asset(ModelAsset *out, Color color);
bool make_wall_asset(ModelAsset *out, float thickness, float height, Color color);

void unload_model_asset(ModelAsset *m);

BoundingBox get_transformed_bbox(Model model, Vector3 pos, Vector3 scale);
BoundingBox get_transformed_bbox_from_asset(const ModelAsset *asset, Vector3 pos, Vector3 scale);

