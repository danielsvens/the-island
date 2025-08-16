#include "assets.h"
#include "raylib.h"

static void apply_atlas_to_all_materials(Model *m, Texture2D atlas) {
    for (int i = 0; i < m->materialCount; ++i) {
        m->materials[i].maps[MATERIAL_MAP_ALBEDO].color = WHITE;
    }
}

Texture2D load_atlas(const char *path) {
    Texture2D t = LoadTexture(path);
    SetTextureFilter(t, TEXTURE_FILTER_BILINEAR);
    return t;
}
void unload_atlas(Texture2D tex) { UnloadTexture(tex); }

bool load_model_with_atlas(ModelAsset *out, const char *obj_path, Texture2D atlas) {
    out->model = LoadModel(obj_path);
    if (out->model.meshCount == 0) return false;
    //apply_atlas_to_all_materials(&out->model, atlas);
    return true;
}

bool load_glb_model(ModelAsset *out, const char *obj_path) {
  out->model = LoadModel(obj_path);
  return out->model.meshCount != 0;
}

static void set_color(Model *m, Color c) {
    for (int i = 0; i < m->materialCount; ++i) {
      m->materials[i].maps[MATERIAL_MAP_ALBEDO].color = c;
    }
}

bool make_plane_asset(ModelAsset *out, Color color) {
    Mesh mesh = GenMeshPlane(1.0f, 1.0f, 1, 1);
    out->model = LoadModelFromMesh(mesh);
    if (out->model.meshCount == 0) return false;
    set_color(&out->model, color);
    return true;
}

bool make_wall_asset(ModelAsset *out, float thickness, float height, Color color) {
    Mesh mesh = GenMeshCube(1.0f, 2.0f, 0.2f);
    out->model = LoadModelFromMesh(mesh);
    if (out->model.meshCount == 0) return false;
    set_color(&out->model, color);
    // We’ll scale per-instance to set thickness/height/length
    return true;
}

void unload_model_asset(ModelAsset *m) { UnloadModel(m->model); }

