#include "animal.h"
#include "../game/game_objects.h"
#include "../game/assets.h"
#include "../core/event_queue.h"
#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>
#include <math.h>

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
  animal->idle_duration = 0.5f + (float)rand() / RAND_MAX * 1.0f; // 0.5-1.5 seconds
  animal->wander_duration = 3.0f + (float)rand() / RAND_MAX * 5.0f; // 3-8 seconds

  // Wandering behavior parameters
  animal->wander_center = Vector3Add(position, (Vector3){2.0f, 0, 0});
  animal->wander_radius = 1.0f;
  animal->wander_angle = (float)rand() / RAND_MAX * 2.0f * PI;
  animal->wander_angle_change = 0.5f;

  // Obstacle avoidance
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
      animal->idle_duration = 1.0f + (float)rand() / RAND_MAX * 4.0f;
      break;
    case AI_STATE_WANDERING:
      animal->wander_duration = 2.0f + (float)rand() / RAND_MAX * 6.0f;
      break;
    case AI_STATE_TURNING:
      animal->target_direction = Vector3Normalize((Vector3){
        (float)rand() / RAND_MAX * 2.0f - 1.0f,
        0,
        (float)rand() / RAND_MAX * 2.0f - 1.0f
      });
      break;
    case AI_STATE_AVOIDING_OBSTACLE:
      break;
  }
}

Vector3 animal_get_wander_force(Animal *animal) {
  animal->wander_angle += ((float)rand() / RAND_MAX * 2.0f - 1.0f) * animal->wander_angle_change;

  Vector3 circle_center = Vector3Add(animal->pos, Vector3Scale(Vector3Normalize(animal->vel), 2.0f));
  Vector3 displacement = {
    cosf(animal->wander_angle) * animal->wander_radius,
    0,
    sinf(animal->wander_angle) * animal->wander_radius
  };

  Vector3 target = Vector3Add(circle_center, displacement);
  Vector3 desired_velocity = Vector3Scale(Vector3Normalize(Vector3Subtract(target, animal->pos)), animal->max_speed);
  return Vector3Subtract(desired_velocity, animal->vel);
}

void animal_handle_ball_collision(Animal *animal, GameObject *ball_obj) {
  Vector3 collision_dir = Vector3Subtract(animal->pos, ball_obj->position);
  collision_dir.y = 0;
  collision_dir = Vector3Normalize(collision_dir);

  float impact_force = Vector3Length(animal->vel) * 2.0f;
  Vector3 ball_impulse = Vector3Scale(collision_dir, -impact_force);


  ball_obj->data.ball.velocity = Vector3Add(ball_obj->data.ball.velocity, ball_impulse);

  Vector3 animal_bounce = Vector3Scale(collision_dir, impact_force * 0.3f);
  animal->vel = Vector3Add(animal->vel, animal_bounce);
}

bool animal_check_collision(const Animal *animal, Vector3 position, const GameWorld *world) {
  // Check collision with world boundaries (simple rectangular boundary)
  float boundary_size = 8.0f;
  if (position.x < -boundary_size || position.x > boundary_size ||
    position.z < -boundary_size || position.z > boundary_size) {
    return true;
  }

  // Create a bounding box for the animal at the test position
  BoundingBox animal_bbox = {
    .min = { position.x - animal->radius, position.y - animal->radius, position.z - animal->radius },
    .max = { position.x + animal->radius, position.y + animal->radius, position.z + animal->radius }
  };

  for (int i = 0; i < world->object_count; i++) {
    const GameObject *obj = &world->objects[i];
    if (!obj->active) continue;

    // Skip if this is the same animal (avoid self-collision)
    if (obj->type == OBJECT_ANIMAL && &obj->data.animal == animal) {
      continue;
    }

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

Vector3 animal_get_avoid_force(Animal *animal, const GameWorld *world) {
  Vector3 avoid_force = {0, 0, 0};
  Vector3 ahead = Vector3Add(animal->pos, Vector3Scale(Vector3Normalize(animal->vel), animal->avoid_distance));

  if (animal_check_collision(animal, ahead, world)) {
    Vector3 right = Vector3CrossProduct(animal->vel, (Vector3){0, 1, 0});
    Vector3 left = Vector3Scale(right, -1.0f);

    Vector3 right_test = Vector3Add(animal->pos, Vector3Scale(Vector3Normalize(right), animal->avoid_distance));
    Vector3 left_test = Vector3Add(animal->pos, Vector3Scale(Vector3Normalize(left), animal->avoid_distance));

    bool right_clear = !animal_check_collision(animal, right_test, world);
    bool left_clear = !animal_check_collision(animal, left_test, world);

    if (right_clear && !left_clear) {
      avoid_force = Vector3Scale(Vector3Normalize(right), animal->max_speed * 2.0f);
    } else if (left_clear && !right_clear) {
      avoid_force = Vector3Scale(Vector3Normalize(left), animal->max_speed * 2.0f);
    } else if (right_clear && left_clear) {
      Vector3 chosen = (rand() % 2) ? right : left;
      avoid_force = Vector3Scale(Vector3Normalize(chosen), animal->max_speed);
    } else {
      avoid_force = Vector3Scale(Vector3Normalize(animal->vel), -animal->max_speed);
    }
  }

  return avoid_force;
}

void animal_update(Animal *animal, float dt, const GameWorld *world) {
  animal->state_timer += dt;

  Vector3 steering_force = {0, 0, 0};

  switch (animal->state) {
    case AI_STATE_IDLE:
      animal->vel = Vector3Scale(animal->vel, 0.95f);

      if (animal->state_timer >= animal->idle_duration) {
        animal_change_state(animal, AI_STATE_WANDERING);
      }
      break;

    case AI_STATE_WANDERING:
      steering_force = animal_get_wander_force(animal);

      // Check for obstacles
      Vector3 avoid_force = animal_get_avoid_force(animal, world);
      if (Vector3Length(avoid_force) > 0.1f) {
        animal_change_state(animal, AI_STATE_AVOIDING_OBSTACLE);
        animal->avoid_force = avoid_force;
      }

      if (animal->state_timer >= animal->wander_duration) {
        if ((float)rand() / RAND_MAX < 0.3f) {
          animal_change_state(animal, AI_STATE_IDLE);
        } else {
          animal_change_state(animal, AI_STATE_TURNING);
        }
      }
      break;

    case AI_STATE_TURNING:
      Vector3 desired_vel = Vector3Scale(animal->target_direction, animal->max_speed * 0.5f);
      steering_force = Vector3Subtract(desired_vel, animal->vel);

      Vector3 current_dir = Vector3Normalize(animal->vel);
      float dot = Vector3DotProduct(current_dir, animal->target_direction);

      if (dot > 0.8f || animal->state_timer > 2.0f) {
        animal_change_state(animal, AI_STATE_WANDERING);
      }
      break;

    case AI_STATE_AVOIDING_OBSTACLE:
      steering_force = animal->avoid_force;

      Vector3 test_pos = Vector3Add(animal->pos, Vector3Scale(Vector3Normalize(animal->vel), 1.0f));
      if (!animal_check_collision(animal, test_pos, world) || animal->state_timer > 3.0f) {
        animal_change_state(animal, AI_STATE_WANDERING);
      }
      break;
  }

  animal->vel = Vector3Add(animal->vel, Vector3Scale(steering_force, dt));

  float speed = Vector3Length(animal->vel);
  if (speed > animal->max_speed) {
    animal->vel = Vector3Scale(Vector3Normalize(animal->vel), animal->max_speed);
  }

  Vector3 new_pos = Vector3Add(animal->pos, Vector3Scale(animal->vel, dt));

  bool blocked = false;
  for (int i = 0; i < world->object_count; i++) {
    GameObject *obj = &((GameWorld*)world)->objects[i];
    if (!obj->active) continue;

    // Skip self
    if (obj->type == OBJECT_ANIMAL && &obj->data.animal == animal) {
      continue;
    }

    BoundingBox animal_bbox = {
      .min = { new_pos.x - animal->radius, new_pos.y - animal->radius, new_pos.z - animal->radius },
      .max = { new_pos.x + animal->radius, new_pos.y + animal->radius, new_pos.z + animal->radius }
    };

    BoundingBox obj_bbox;
    if (obj->model && obj->model->model.meshCount > 0) {
      obj_bbox = get_transformed_bbox(obj->model->model, obj->position, obj->scale);
    } else {
      break;
    }

    if (CheckCollisionBoxes(animal_bbox, obj_bbox)) {
      if (obj->type == OBJECT_BALL) {
        animal_handle_ball_collision(animal, obj);
      } else {
        // Solid obstacle - block movement
        blocked = true;
        break;
      }
    }
  }

  // Check world boundaries
  float boundary_size = 8.0f;
  if (new_pos.x < -boundary_size || new_pos.x > boundary_size ||
    new_pos.z < -boundary_size || new_pos.z > boundary_size) {
    blocked = true;
  }

  if (!blocked) {
    animal->pos = new_pos;
  } else {
    // Stop and change direction
    animal->vel = Vector3Scale(animal->vel, -0.5f);
    animal_change_state(animal, AI_STATE_AVOIDING_OBSTACLE);
  }

  animal->pos.y = 0.5f;
}
