#pragma once
#include <raylib.h>
#include "assets.h"
#include "../ai/animal.h"

typedef enum {
  OBJECT_STATIC,
  OBJECT_BALL,
  OBJECT_TREE,
  OBJECT_ANIMAL,
} ObjectType;

typedef struct PhysicsBody {
  Vector3 velocity;
  float mass;
  bool use_gravity;
} PhysicsBody;

typedef struct {
  PhysicsBody phys;
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

typedef struct GameObject {
  int id;
  ObjectType type;
  Vector3 position;
  Vector3 rotation;
  Vector3 scale;
  ModelAsset *model;
  Color tint;
  bool active;

  union {
    BallData ball;
    TreeData tree;
    Animal animal;
  } data;
} GameObject;

#define MAX_OBJECTS 500

typedef struct GameWorld {
  GameObject objects[MAX_OBJECTS];
  int object_count;
  int next_id;
} GameWorld;

// World management
void world_init(GameWorld *world);
void world_shutdown(GameWorld *world);
void world_destroy_object(GameWorld *world, int id);

int world_create_object(GameWorld *world, ObjectType type);
GameObject *world_get_object(GameWorld *world, int id);

int create_ball(GameWorld *world, Vector3 position, ModelAsset *model, 
                float radius, float mass, float bounce, float friction);
int create_tree(GameWorld *world, Vector3 position, ModelAsset *model,
                float sway_speed, float sway_amount);
int create_static_object(GameWorld *world, Vector3 position, ModelAsset *model);
int create_animal(GameWorld *world, Vector3 position, ModelAsset *model, float max_speed);

void update_physics(GameWorld *world, float dt);
void update_trees(GameWorld *world, float dt);

void check_collisions(GameWorld *world);
void render_objects(const GameWorld *world);
