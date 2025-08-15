
#include "lighting.h"
#include <string.h>
#include "rlgl.h"

#define RLIGHTS_IMPLEMENTATION
#include "rlights.h"

bool lighting_init(Lighting* L, const float ambientRGBA[4], Vector3 dir, Color dirColor) {
    memset(L, 0, sizeof(*L));

    L->shader = LoadShader("shaders/lighting.vs", "shaders/lighting.fs");
    if (L->shader.id == 0) return false;

    L->locViewPos = GetShaderLocation(L->shader, "viewPos");
    L->locAmbient = GetShaderLocation(L->shader, "ambient");
    SetShaderValue(L->shader, L->locAmbient, ambientRGBA, SHADER_UNIFORM_VEC4);

    Light *light0 = (Light*) MemAlloc(sizeof(*light0));
    *light0 = CreateLight(LIGHT_DIRECTIONAL, (Vector3){0,0,0}, dir, dirColor, L->shader);
    L->_light0 = light0;
    return true;
}

void lighting_apply_to_model(Lighting* L, Model* m, Color tint) {
    if (!L || !m) return;
    for (int i = 0; i < m->materialCount; ++i) {
      m->materials[i].shader = L->shader;
      m->materials[i].maps[MATERIAL_MAP_ALBEDO].color = tint;
    }
}

void lighting_update(Lighting* L, const Camera* cam) {
    if (!L || !cam) return;
    SetShaderValue(L->shader, L->locViewPos, (void*)&cam->position, SHADER_UNIFORM_VEC3);
    if (L->_light0) UpdateLightValues(L->shader, *(Light*)L->_light0);
}

void lighting_shutdown(Lighting* L) {
    if (!L) return;
    if (L->_light0) { MemFree(L->_light0); L->_light0 = NULL; }
    if (L->shader.id) UnloadShader(L->shader);
    memset(L, 0, sizeof(*L));
}
