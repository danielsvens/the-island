#include "scene.h"

void scene_reset(Scene *s) { s->count = 0; }

int scene_add(Scene *s, const ModelAsset *asset, Vector3 pos, Vector3 scale,
              Vector3 rotAxis, float rotAngleDeg, Color tint) {
    if (s->count >= MAX_INSTANCES) return -1;
    s->inst[s->count++] = (Instance){ asset, pos, scale, rotAxis, rotAngleDeg, tint };
    return s->count - 1;
}

