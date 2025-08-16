#include "game_objects.h"
#include <raylib.h>
#include <string.h>
#include "math.h"

void world_init(GameWorld *world) {
    memset(world, 0, sizeof(GameWorld));
    world->next_id = 1;
}

void world_shutdown(GameWorld *world) {
    world->object_count = 0;
    world->next_id = 1;
}

int world_create_object(GameWorld *world, ObjectType type) {
    if (world->object_count >= MAX_OBJECTS) {
        TraceLog(LOG_WARNING, "Cannot create object: world full");
        return 0;
    }
    
    GameObject *obj = &world->objects[world->object_count];
    memset(obj, 0, sizeof(GameObject));
    
    obj->id = world->next_id++;
    obj->type = type;
    obj->active = true;
    obj->scale = (Vector3){1.0f, 1.0f, 1.0f};
    obj->tint = WHITE;
    
    world->object_count++;
    return obj->id;
}

GameObject *world_get_object(GameWorld *world, int id) {
    for (int i = 0; i < world->object_count; i++) {
        if (world->objects[i].id == id && world->objects[i].active) {
            return &world->objects[i];
        }
    }
    return NULL;
}

void world_destroy_object(GameWorld *world, int id) {
    for (int i = 0; i < world->object_count; i++) {
        if (world->objects[i].id == id) {
            // Move last object to this slot
            world->objects[i] = world->objects[world->object_count - 1];
            world->object_count--;
            break;
        }
    }
}

int create_ball(GameWorld *world, Vector3 position, ModelAsset *model, 
                     float radius, float mass, float bounce, float friction) {
    int id = world_create_object(world, OBJECT_BALL);
    GameObject *obj = world_get_object(world, id);
    if (!obj) return 0;
    
    obj->position = position;
    obj->model = model;
    obj->data.ball.velocity = (Vector3){0, 0, 0};
    obj->data.ball.radius = radius;
    obj->data.ball.mass = mass;
    obj->data.ball.bounce_factor = bounce;
    obj->data.ball.friction = friction;
    obj->data.ball.use_gravity = true;
    
    return id;
}

int create_tree(GameWorld *world, Vector3 position, ModelAsset *model,
                     float sway_speed, float sway_amount) {
    int id = world_create_object(world, OBJECT_TREE);
    GameObject *obj = world_get_object(world, id);
    if (!obj) return 0;
    
    obj->position = position;
    obj->model = model;
    obj->data.tree.sway_phase = 0.0f;
    obj->data.tree.sway_speed = sway_speed;
    obj->data.tree.sway_amount = sway_amount;
    obj->data.tree.height = 1.0f;
    
    return id;
}

int create_static_object(GameWorld *world, Vector3 position, ModelAsset *model) {
    int id = world_create_object(world, OBJECT_STATIC);
    GameObject *obj = world_get_object(world, id);
    if (!obj) return 0;
    
    obj->position = position;
    obj->model = model;
    
    return id;
}

void update_physics(GameWorld *world, float dt) {
    const float gravity = -9.81f;
    const float ground_y = 0.0f;
    
    for (int i = 0; i < world->object_count; i++) {
        GameObject *obj = &world->objects[i];
        if (!obj->active || obj->type != OBJECT_BALL) continue;
        
        BallData *ball = &obj->data.ball;
        
        // Apply gravity
        if (ball->use_gravity) {
            ball->velocity.y += gravity * dt;
        }

        obj->position.x += ball->velocity.x * dt;
        obj->position.y += ball->velocity.y * dt;
        obj->position.z += ball->velocity.z * dt;

        if (obj->position.y - ball->radius <= ground_y) {
            obj->position.y = ground_y + ball->radius;
            ball->velocity.y = -ball->velocity.y * ball->bounce_factor;
            
            ball->velocity.x *= (1.0f - ball->friction * dt);
            ball->velocity.z *= (1.0f - ball->friction * dt);
        }
    }
}

void update_trees(GameWorld *world, float dt) {
    for (int i = 0; i < world->object_count; i++) {
        GameObject *obj = &world->objects[i];
        if (!obj->active || obj->type != OBJECT_TREE) continue;
        
        TreeData *tree = &obj->data.tree;
        
        // Update sway animation
        tree->sway_phase += tree->sway_speed * dt;
        
        // Apply sway to rotation
        obj->rotation.z = sinf(tree->sway_phase) * tree->sway_amount;
    }
}

void render_objects(const GameWorld *world) {
    for (int i = 0; i < world->object_count; i++) {
        const GameObject *obj = &world->objects[i];
        if (!obj->active || !obj->model) continue;

        DrawModelEx(obj->model->model, obj->position, obj->rotation, obj->rotation_axis, obj->scale, obj->tint);
    }
}