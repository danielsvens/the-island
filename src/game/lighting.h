#pragma once
#include <raylib.h>

typedef struct {
    Shader shader;
    int loc_view_pos;
    int loc_ambient;

    void *_light0;
    void *_light1;
} Lighting;

bool lighting_init(Lighting *L, const float ambient_rgba[4], Vector3 dir, Color dir_color);
void lighting_apply_to_model(Lighting *L, Model *m, Color tint);
void lighting_update(Lighting *L, const Camera *cam);
void lighting_shutdown(Lighting *L);
