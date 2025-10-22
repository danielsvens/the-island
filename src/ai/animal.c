#include "animal.h"
#include "../game/game_objects.h"
#include "../game/assets.h"
#include "../core/event_queue.h"
#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

static inline float v3_len2(Vector3 v) { return v.x * v.x + v.y * v.y + v.z * v.z; }
static inline Vector3 v3_add(Vector3 a, Vector3 b){ return (Vector3){a.x + b.x, a.y + b.y, a.z + b.z}; }
static inline Vector3 v3_sub(Vector3 a, Vector3 b){ return (Vector3){a.x - b.x, a.y - b.y, a.z - b.z}; }
static inline Vector3 v3_scale(Vector3 a, float s){ return (Vector3){a.x * s, a.y * s, a.z * s}; }

static inline Vector3 safe_normalize(Vector3 v) {
  float l2 = v3_len2(v);
  if (l2 < 1e-8f) return (Vector3){0,0,0};
  float inv = 1.0f / sqrtf(l2);
  return (Vector3){v.x * inv, v.y * inv, v.z * inv};
}

void animal_init(Animal *animal, Vector3 position, float max_speed) {
  animal->pos = position;
  animal->vel = (Vector3){0, 0, 0};
  animal->target_direction = (Vector3){1, 0, 0};

  animal->max_speed = max_speed;
  animal->radius = 0.5f;
  animal->turn_speed = 2.0f;
  animal->detection_radius = 3.0f;

  animal->state = AI_STATE_WANDERING;
  animal->state_timer = 0.0f;
  animal->idle_duration   = 0.5f + (float)rand()/RAND_MAX * 1.0f; // 0.5–1.5s
  animal->wander_duration = 3.0f + (float)rand()/RAND_MAX * 5.0f; // 3–8s

  animal->wander_center = Vector3Add(position, (Vector3){2.0f, 0, 0});
  animal->wander_radius = 1.0f;
  animal->wander_angle = (float)rand()/RAND_MAX * 2.0f * PI;
  animal->wander_angle_change = 0.5f;

  animal->avoid_force = (Vector3){0, 0, 0};
  animal->avoid_distance = 1.0f;
}

void animal_change_state(Animal *animal, AIState new_state) {
  AIState old_state = animal->state;

  Event event = make_event(EVENT_AI_STATE_CHANGE);

  event.ai_state_change.animal_ptr = animal;
  event.ai_state_change.old_state = (int)old_state;
  event.ai_state_change.new_state = (int)new_state;
  push_event(&event);

  animal->state = new_state;
  animal->state_timer = 0.0f;

  switch (new_state) {
    case AI_STATE_IDLE:
      animal->idle_duration = 1.0f + (float)rand()/RAND_MAX * 4.0f;
      break;
    case AI_STATE_WANDERING:
      animal->wander_duration = 2.0f + (float)rand()/RAND_MAX * 6.0f;
      break;
    case AI_STATE_TURNING:
      animal->target_direction = safe_normalize((Vector3){
        (float)rand()/RAND_MAX * 2.0f - 1.0f,
        0.0f,
        (float)rand()/RAND_MAX * 2.0f - 1.0f
      });
      break;
    case AI_STATE_AVOIDING_OBSTACLE:
      break;
  }
}

Vector3 animal_get_wander_force(Animal *animal) {
  animal->wander_angle += ((float) rand() / RAND_MAX * 2.0f - 1.0f) * animal->wander_angle_change;

  Vector3 fwd = safe_normalize(animal->vel);
  if (v3_len2(fwd) < 1e-8f) fwd = (Vector3){1,0,0}; // default forward when still

  Vector3 circle_center = Vector3Add(animal->pos, Vector3Scale(fwd, 2.0f));
  Vector3 displacement = (Vector3){
    cosf(animal->wander_angle) * animal->wander_radius,
    0.0f,
    sinf(animal->wander_angle) * animal->wander_radius
  };

  Vector3 target = Vector3Add(circle_center, displacement);
  Vector3 to_target = Vector3Subtract(target, animal->pos);
  Vector3 desired_velocity = Vector3Scale(safe_normalize(to_target), animal->max_speed);
  return Vector3Subtract(desired_velocity, animal->vel);
}

void animal_handle_ball_collision(Animal *animal, GameObject *ball_obj) {
  Vector3 collision_dir = Vector3Subtract(animal->pos, ball_obj->position);
  collision_dir.y = 0.0f;
  collision_dir = safe_normalize(collision_dir);

  float impact_force = sqrtf(v3_len2(animal->vel)) * 2.0f;
  Vector3 ball_impulse = v3_scale(collision_dir, -impact_force);

  ball_obj->data.ball.velocity = v3_add(ball_obj->data.ball.velocity, ball_impulse);

  Vector3 animal_bounce = v3_scale(collision_dir, impact_force * 0.3f);
  animal->vel = v3_add(animal->vel, animal_bounce);
}

bool animal_check_collision(const Animal *animal, Vector3 position, const GameWorld *world) {
  const float boundary_size = 8.0f;
  if (position.x < -boundary_size || position.x > boundary_size ||
    position.z < -boundary_size || position.z > boundary_size) {
    return true;
  }

  BoundingBox animal_bbox = (BoundingBox){
    .min = { position.x - animal->radius, position.y - animal->radius, position.z - animal->radius },
    .max = { position.x + animal->radius, position.y + animal->radius, position.z + animal->radius }
  };

  for (int i = 0; i < world->object_count; ++i) {
    const GameObject *obj = &world->objects[i];
    if (!obj->active) continue;
    if (obj->type == OBJECT_ANIMAL && &obj->data.animal == animal) continue;

    BoundingBox obj_bbox;
    if (obj->model && obj->model->model.meshCount > 0) {
      obj_bbox = get_transformed_bbox_from_asset(obj->model, obj->position, obj->scale);
    } else {
      float default_size = 0.5f;
      obj_bbox = (BoundingBox){
        .min = { obj->position.x - default_size, obj->position.y - default_size, obj->position.z - default_size },
        .max = { obj->position.x + default_size, obj->position.y + default_size, obj->position.z + default_size }
      };
    }

    if (CheckCollisionBoxes(animal_bbox, obj_bbox)) {
      return true;
    }
  }

  return false;
}

static bool animal_query_collision(const Animal *animal, Vector3 position,
                                   const GameWorld *world,
                                   const GameObject **out_hit_obj) {
  const float boundary_size = 8.0f;
  if (position.x < -boundary_size || position.x > boundary_size ||
    position.z < -boundary_size || position.z > boundary_size) {
    if (out_hit_obj) *out_hit_obj = NULL; // boundary hit
    return true;
  }

  BoundingBox animal_bbox = (BoundingBox){
    .min = { position.x - animal->radius, position.y - animal->radius, position.z - animal->radius },
    .max = { position.x + animal->radius, position.y + animal->radius, position.z + animal->radius }
  };

  for (int i = 0; i < world->object_count; ++i) {
    const GameObject *obj = &world->objects[i];

    if (!obj->active) continue;
    if (obj->type == OBJECT_ANIMAL && &obj->data.animal == animal) continue;

    BoundingBox obj_bbox;

    if (obj->model && obj->model->model.meshCount > 0) {
      obj_bbox = get_transformed_bbox_from_asset(obj->model, obj->position, obj->scale);
    } else {
      float default_size = 0.5f;
      obj_bbox = (BoundingBox){
        .min = { obj->position.x - default_size, obj->position.y - default_size, obj->position.z - default_size },
        .max = { obj->position.x + default_size, obj->position.y + default_size, obj->position.z + default_size }
      };
    }

    if (CheckCollisionBoxes(animal_bbox, obj_bbox)) {
      if (out_hit_obj) *out_hit_obj = obj;
      return true;
    }
  }

  if (out_hit_obj) *out_hit_obj = NULL;
  return false;
}

Vector3 animal_get_avoid_force(Animal *animal, const GameWorld *world) {
  Vector3 vnorm = safe_normalize(animal->vel);
  if (v3_len2(vnorm) < 1e-8f) return (Vector3){0}; // not moving → nothing to avoid

  Vector3 ahead = v3_add(animal->pos, v3_scale(vnorm, animal->avoid_distance));
  if (!animal_check_collision(animal, ahead, world)) return (Vector3){0};

  Vector3 right = safe_normalize(Vector3CrossProduct(vnorm, (Vector3){0,1,0}));
  if (v3_len2(right) < 1e-8f) right = (Vector3){1,0,0};
  Vector3 left  = (Vector3){-right.x, -right.y, -right.z};

  Vector3 side = (rand() & 1) ? right : left;

  return v3_scale(side, animal->max_speed * 2.0f);
}

void animal_update(Animal *animal, float dt, const GameWorld *world) {
  animal->state_timer += dt;

  Vector3 steering_force = (Vector3){0,0,0};

  switch (animal->state) {
    case AI_STATE_IDLE:
      animal->vel = v3_scale(animal->vel, 0.95f);

      if (animal->state_timer >= animal->idle_duration) {
        animal_change_state(animal, AI_STATE_WANDERING);
      }

      break;

    case AI_STATE_WANDERING: 
      steering_force = animal_get_wander_force(animal);

      Vector3 avoid_force = animal_get_avoid_force(animal, world);
      if (v3_len2(avoid_force) > 0.01f) {
        animal->avoid_force = avoid_force;
        animal_change_state(animal, AI_STATE_AVOIDING_OBSTACLE);
      }

      if (animal->state_timer >= animal->wander_duration) {
        if ((float)rand()/RAND_MAX < 0.3f) animal_change_state(animal, AI_STATE_IDLE);
        else                               animal_change_state(animal, AI_STATE_TURNING);
      }
      break;

    case AI_STATE_TURNING:
      Vector3 desired_vel = v3_scale(animal->target_direction, animal->max_speed * 0.5f);
      steering_force = v3_sub(desired_vel, animal->vel);

      Vector3 current_dir = safe_normalize(animal->vel);
      float dot = Vector3DotProduct(current_dir, animal->target_direction);
      if (dot > 0.8f || animal->state_timer > 2.0f)
        animal_change_state(animal, AI_STATE_WANDERING);
      break;

    case AI_STATE_AVOIDING_OBSTACLE:
      steering_force = animal->avoid_force;

      Vector3 vnorm = safe_normalize(animal->vel);
      if (v3_len2(vnorm) > 0.0f) {
        Vector3 test_pos = v3_add(animal->pos, v3_scale(vnorm, 1.0f));
        if (!animal_check_collision(animal, test_pos, world) || animal->state_timer > 3.0f)
          animal_change_state(animal, AI_STATE_WANDERING);
      } else if (animal->state_timer > 0.5f) {
        animal_change_state(animal, AI_STATE_WANDERING);
      }
      break;
  }

  float gravity = -9.81f;
  steering_force.y += gravity;

  animal->vel = v3_add(animal->vel, v3_scale(steering_force, dt));

  float sp2 = v3_len2(animal->vel);
  float max2 = animal->max_speed * animal->max_speed;
  if (sp2 > max2) {
    float inv = animal->max_speed / sqrtf(sp2);
    animal->vel = v3_scale(animal->vel, inv);
  }

  Vector3 new_pos = v3_add(animal->pos, v3_scale(animal->vel, dt));

  const GameObject *hit = NULL;
  bool blocked = animal_query_collision(animal, new_pos, world, &hit);

  if (!blocked) {
    animal->pos = new_pos;
  } else {
    if (hit && hit->type == OBJECT_BALL) {
      animal_handle_ball_collision(animal, (GameObject*)hit);
    }
    animal->vel = v3_scale(animal->vel, -0.5f);
    animal_change_state(animal, AI_STATE_AVOIDING_OBSTACLE);
  }

  animal->pos.y = 0.0f;
  if (animal->pos.y == 0.0f && animal->vel.y < 0.0f) animal->vel.y = 0.0f;
}
