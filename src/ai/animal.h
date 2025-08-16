#pragma once
#include <raylib.h>
#include <stdbool.h>

// Forward declarations to avoid circular dependency
typedef struct GameWorld GameWorld;
typedef struct GameObject GameObject;

typedef enum {
    AI_STATE_IDLE,
    AI_STATE_WANDERING,
    AI_STATE_TURNING,
    AI_STATE_AVOIDING_OBSTACLE
} AIState;

typedef struct {
    Vector3 pos;
    Vector3 vel;
    Vector3 target_direction;
    
    float max_speed;
    float radius;
    float turn_speed;
    float detection_radius;
    
    AIState state;
    float state_timer;
    float idle_duration;
    float wander_duration;
    
    // Wandering behavior
    Vector3 wander_center;
    float wander_radius;
    float wander_angle;
    float wander_angle_change;
    
    // Obstacle avoidance
    Vector3 avoid_force;
    float avoid_distance;
} Animal;

// Function declarations
void animal_init(Animal *animal, Vector3 position, float max_speed);
void animal_update(Animal *animal, float dt, const GameWorld *world);

Vector3 animal_get_wander_force(Animal *animal);
Vector3 animal_get_avoid_force(Animal *animal, const GameWorld *world);

bool animal_check_collision(const Animal *animal, Vector3 position, const GameWorld *world);
void animal_change_state(Animal *animal, AIState new_state);
void animal_handle_ball_collision(Animal *animal, GameObject *ball_obj);
