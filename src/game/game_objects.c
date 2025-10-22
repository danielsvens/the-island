#include "game_objects.h"
#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>
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
  obj->rotation = (Vector3){0.0f, 0.0f, 0.0f};  // No rotation by default
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
  obj->data.ball.orientation = QuaternionIdentity();
  obj->data.ball.ang_vel = (Vector3) { 0, 0, 0 };

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

int create_animal(GameWorld *world, Vector3 position, ModelAsset *model, float max_speed) {
  int id = world_create_object(world, OBJECT_ANIMAL);
  GameObject *obj = world_get_object(world, id);
  if (!obj) return 0;

  obj->position = position;
  obj->model = model;
  obj->rotation = (Vector3){90.0f, 180.0f, 0.0f};

  animal_init(&obj->data.animal, position, max_speed);

  return id;
}

static void handle_ball_collision(GameObject *ball1, GameObject *ball2) {
  BallData *b1 = &ball1->data.ball;
  BallData *b2 = &ball2->data.ball;

  Vector3 collision_normal = Vector3Normalize(Vector3Subtract(ball2->position, ball1->position));
  Vector3 relative_velocity = Vector3Subtract(b2->velocity, b1->velocity);
  float velocity_along_normal = Vector3DotProduct(relative_velocity, collision_normal);

  if (velocity_along_normal > 0) return;

  float restitution = (b1->bounce_factor + b2->bounce_factor) * 0.5f;
  float impulse_scalar = -(1 + restitution) * velocity_along_normal;
  impulse_scalar /= (1/b1->mass + 1/b2->mass);

  Vector3 impulse = Vector3Scale(collision_normal, impulse_scalar);

  b1->velocity = Vector3Subtract(b1->velocity, Vector3Scale(impulse, 1/b1->mass));
  b2->velocity = Vector3Add(b2->velocity, Vector3Scale(impulse, 1/b2->mass));

  float overlap = (b1->radius + b2->radius) - Vector3Distance(ball1->position, ball2->position);
  if (overlap > 0) {
    Vector3 separation = Vector3Scale(collision_normal, overlap * 0.5f);
    ball1->position = Vector3Subtract(ball1->position, separation);
    ball2->position = Vector3Add(ball2->position, separation);
  } 
}

static void handle_ball_static_collision(GameObject *ball, GameObject *static_obj) {
  BallData *ball_data = &ball->data.ball;

  BoundingBox ball_bbox = {
    .min = {ball->position.x - ball_data->radius, ball->position.y - ball_data->radius, ball->position.z - ball_data->radius},
    .max = {ball->position.x + ball_data->radius, ball->position.y + ball_data->radius, ball->position.z + ball_data->radius}
  };

  BoundingBox static_bbox = get_transformed_bbox_from_asset(static_obj->model, static_obj->position, static_obj->scale);

  if (CheckCollisionBoxes(ball_bbox, static_bbox)) {
    Vector3 ball_center = ball->position;
    Vector3 static_center = Vector3Scale(Vector3Add(static_bbox.min, static_bbox.max), 0.5f);
    Vector3 collision_normal = Vector3Normalize(Vector3Subtract(ball_center, static_center));

    float dot = Vector3DotProduct(ball_data->velocity, collision_normal);
    ball_data->velocity = Vector3Subtract(ball_data->velocity, Vector3Scale(collision_normal, 2.0f * dot));
    ball_data->velocity = Vector3Scale(ball_data->velocity, ball_data->bounce_factor);

    float penetration = ball_data->radius - Vector3Distance(ball_center, static_center);
    if (penetration > 0) {
      ball->position = Vector3Add(ball->position, Vector3Scale(collision_normal, penetration));
    }
  }
}

void check_collisions(GameWorld *world) {
  for (int i = 0; i < world->object_count; i++) {
    GameObject *obj1 = &world->objects[i];
    if (!obj1->active || obj1->type != OBJECT_BALL) continue;

    for (int j = i + 1; j < world->object_count; j++) {
      GameObject *obj2 = &world->objects[j];
      if (!obj2->active) continue;

      if (obj2->type == OBJECT_BALL) {
        // Ball-to-ball collision
        float distance = Vector3Distance(obj1->position, obj2->position);
        float min_distance = obj1->data.ball.radius + obj2->data.ball.radius;

        if (distance < min_distance) {
          handle_ball_collision(obj1, obj2);
        }
      } else if (obj2->type == OBJECT_STATIC || obj2->type == OBJECT_TREE) {
        // Ball-to-static collision
        handle_ball_static_collision(obj1, obj2);
      }
    }
  }
}

void update_physics(GameWorld *world, float dt) {
  const float gravity = -9.81f;
  const float ground_y = 0.0f;

  for (int i = 0; i < world->object_count; i++) {
    GameObject *obj = &world->objects[i];
    if (!obj->active) continue;

    switch (obj->type) {
      case OBJECT_BALL:
        BallData *ball = &obj->data.ball;

        if (ball->use_gravity) {
          ball->velocity.y += gravity * dt;
        }

        obj->position.x += ball->velocity.x * dt;
        obj->position.y += ball->velocity.y * dt;
        obj->position.z += ball->velocity.z * dt;

        // Ground collision
        if (obj->position.y - ball->radius <= ground_y) {
          obj->position.y = ground_y + ball->radius;
          ball->velocity.y = -ball->velocity.y * ball->bounce_factor;

          float k = expf(-ball->friction * dt);
          ball->velocity.x *= k;
          ball->velocity.z *= k;
        }

        break;

      case OBJECT_ANIMAL:
        animal_update(&obj->data.animal, dt, world);
        obj->position = obj->data.animal.pos;
        break;
      case OBJECT_STATIC:
      case OBJECT_TREE:
        break;
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

    // Apply sway to rotation (rotate around Z-axis for sway effect)
    obj->rotation.z = sinf(tree->sway_phase) * tree->sway_amount;
  }
}

void render_objects(const GameWorld *world) {
  for (int i = 0; i < world->object_count; i++) {
    const GameObject *obj = &world->objects[i];
    if (!obj->active || !obj->model) continue;

    rlPushMatrix();
    rlTranslatef(obj->position.x, obj->position.y, obj->position.z);
    rlRotatef(obj->rotation.x, 1, 0, 0);
    rlRotatef(obj->rotation.y, 0, 1, 0);
    rlRotatef(obj->rotation.z, 0, 0, 1);
    rlScalef(obj->scale.x, obj->scale.y, obj->scale.z);

    DrawModel(obj->model->model, (Vector3){0, 0, 0}, 1.0f, obj->tint);
    rlPopMatrix();
  }
}
