#include "scene.h"

void scene_reset(Scene *s) { s->count = 0; }

int scene_add(Scene *s, const ModelAsset *asset, Vector3 pos, Vector3 scale,
              Vector3 rot_axis, float rot_angle_deg, Color tint) {
    if (s->count >= MAX_INSTANCES) return -1;
    s->inst[s->count++] = (Instance){ asset, pos, scale, rot_axis, rot_angle_deg, tint };
    return s->count - 1;
}

void render_scene(const Scene *s) {
    for (int i = 0; i < s->count; ++i) {
        const Instance *it = &s->inst[i];
        DrawModelEx(it->asset->model, it->pos, it->rot_axis, it->rot_angle_deg, it->scale, it->tint);
        //DrawModelWiresEx(it->asset->model, it->pos, it->rotAxis, it->rotAngleDeg, it->scale, BLACK);

        // draw grid
        //DrawBoundingBox(get_transformed_bbox(g->model, g->position, g->scale), RED);
    }
}

