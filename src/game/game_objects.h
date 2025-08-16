#pragma once
#include <raylib.h>
#include "assets.h"

typedef enum {
    OBJECT_STATIC,
    OBJECT_BALL,
    OBJECT_TREE,
} ObjectType;

typedef struct {
    Vector3 velocity;
    float radius;
    float bounce_factor;
    float friction;
    float mass;
    bool use_gravity;
} BallData;

typedef struct {
    float sway_phase;
    float sway_speed;
    float sway_amount;
    float height;
} TreeData;

typedef struct {
    int id;
    ObjectType type;
    Vector3 position;
    Vector3 rotation;
    float rotation_axis;
    Vector3 scale;
    ModelAsset *model;
    Color tint;
    bool active;
    
    union {
        BallData ball;
        TreeData tree;
    } data;
} GameObject;

#define MAX_OBJECTS 500

typedef struct {
    GameObject objects[MAX_OBJECTS];
    int object_count;
    int next_id;
} GameWorld;

// World management
void world_init(GameWorld *world);
void world_shutdown(GameWorld *world);
int world_create_object(GameWorld *world, ObjectType type);
GameObject *world_get_object(GameWorld *world, int id);
void world_destroy_object(GameWorld *world, int id);

int create_ball(GameWorld *world, Vector3 position, ModelAsset *model, 
                     float radius, float mass, float bounce, float friction);
int create_tree(GameWorld *world, Vector3 position, ModelAsset *model,
                     float sway_speed, float sway_amount);
int create_static_object(GameWorld *world, Vector3 position, ModelAsset *model);

// Update systems
void update_physics(GameWorld *world, float dt);
void update_trees(GameWorld *world, float dt);
void render_objects(const GameWorld *world);