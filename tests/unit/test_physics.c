// Tests for the Physics & Collision System

#define PAL_MOCK_ENABLED
#include "../test_framework.h"
#include "engine/physics.h"
#include "engine/engine.h"
#include "engine/engine_natives.h"
#include "runtime/stdlib.h"
#include "vm/vm.h"
#include "vm/gc.h"
#include "pal/pal.h"
#include <math.h>

// Helper to compare floats with epsilon
#define FLOAT_EQ(a, b) (fabs((a) - (b)) < 0.0001)

// Helper to set up test environment with VM and sprites
static VM test_vm;
static Engine* test_engine;

static void setup_test_env(void) {
    vm_init(&test_vm);
    gc_set_vm(&test_vm);
    stdlib_init(&test_vm);
    test_engine = engine_new(&test_vm);
    engine_set(test_engine);
    engine_init(test_engine, PAL_BACKEND_MOCK);
    engine_natives_init(&test_vm);
}

static void teardown_test_env(void) {
    engine_shutdown(test_engine);
    engine_free(test_engine);
    vm_free(&test_vm);
    gc_set_vm(NULL);
}

// ============================================================================
// Math Helper Tests
// ============================================================================

TEST(lerp_basic) {
    // Lerp from 0 to 10
    ASSERT(FLOAT_EQ(physics_lerp(0, 10, 0.0), 0.0));
    ASSERT(FLOAT_EQ(physics_lerp(0, 10, 0.5), 5.0));
    ASSERT(FLOAT_EQ(physics_lerp(0, 10, 1.0), 10.0));
}

TEST(lerp_clamps) {
    // Values below 0 and above 1 should be clamped
    ASSERT(FLOAT_EQ(physics_lerp(0, 10, -1.0), 0.0));
    ASSERT(FLOAT_EQ(physics_lerp(0, 10, 2.0), 10.0));
}

TEST(lerp_negative) {
    // Lerp with negative values
    ASSERT(FLOAT_EQ(physics_lerp(-10, 10, 0.5), 0.0));
    ASSERT(FLOAT_EQ(physics_lerp(10, -10, 0.5), 0.0));
}

TEST(normalize_angle) {
    ASSERT(FLOAT_EQ(physics_normalize_angle(0), 0.0));
    ASSERT(FLOAT_EQ(physics_normalize_angle(180), 180.0));
    ASSERT(FLOAT_EQ(physics_normalize_angle(360), 0.0));
    ASSERT(FLOAT_EQ(physics_normalize_angle(450), 90.0));
    ASSERT(FLOAT_EQ(physics_normalize_angle(-90), 270.0));
    ASSERT(FLOAT_EQ(physics_normalize_angle(-180), 180.0));
}

TEST(lerp_angle_basic) {
    // Simple angle interpolation
    ASSERT(FLOAT_EQ(physics_lerp_angle(0, 90, 0.5), 45.0));
    ASSERT(FLOAT_EQ(physics_lerp_angle(0, 180, 0.5), 90.0));
}

TEST(lerp_angle_wraparound) {
    // Wraparound cases (should take shortest path)
    // From 350 to 10 should go through 0, not backwards
    double result = physics_lerp_angle(350, 10, 0.5);
    ASSERT(FLOAT_EQ(result, 0.0));

    // From 10 to 350 should also go through 0
    result = physics_lerp_angle(10, 350, 0.5);
    ASSERT(FLOAT_EQ(result, 0.0));
}

// ============================================================================
// Gravity Tests
// ============================================================================

TEST(gravity_default) {
    // Default gravity should be 0
    ASSERT(FLOAT_EQ(physics_get_gravity(), 0.0));
}

TEST(gravity_set_get) {
    physics_set_gravity(500.0);
    ASSERT(FLOAT_EQ(physics_get_gravity(), 500.0));

    physics_set_gravity(-100.0);
    ASSERT(FLOAT_EQ(physics_get_gravity(), -100.0));

    // Reset
    physics_set_gravity(0.0);
}

// ============================================================================
// Collision Detection Tests - AABB
// ============================================================================

TEST(collides_overlapping) {
    setup_test_env();

    // Create two overlapping sprites
    ObjSprite* a = sprite_new(NULL);
    a->x = 100;
    a->y = 100;
    a->width = 50;
    a->height = 50;

    ObjSprite* b = sprite_new(NULL);
    b->x = 120;
    b->y = 120;
    b->width = 50;
    b->height = 50;

    ASSERT(physics_collides(a, b));

    teardown_test_env();
}

TEST(collides_not_overlapping) {
    setup_test_env();

    ObjSprite* a = sprite_new(NULL);
    a->x = 0;
    a->y = 0;
    a->width = 50;
    a->height = 50;

    ObjSprite* b = sprite_new(NULL);
    b->x = 200;
    b->y = 200;
    b->width = 50;
    b->height = 50;

    ASSERT(!physics_collides(a, b));

    teardown_test_env();
}

TEST(collides_touching) {
    setup_test_env();

    // Sprites exactly touching (edge case)
    ObjSprite* a = sprite_new(NULL);
    a->x = 0;
    a->y = 0;
    a->width = 50;
    a->height = 50;

    ObjSprite* b = sprite_new(NULL);
    b->x = 50;
    b->y = 0;
    b->width = 50;
    b->height = 50;

    // Touching but not overlapping should be false
    ASSERT(!physics_collides(a, b));

    teardown_test_env();
}

TEST(collides_rect) {
    setup_test_env();

    ObjSprite* sprite = sprite_new(NULL);
    sprite->x = 100;
    sprite->y = 100;
    sprite->width = 50;
    sprite->height = 50;

    // Overlapping rect
    ASSERT(physics_collides_rect(sprite, 90, 90, 30, 30));

    // Non-overlapping rect
    ASSERT(!physics_collides_rect(sprite, 0, 0, 30, 30));

    teardown_test_env();
}

TEST(collides_point) {
    setup_test_env();

    ObjSprite* sprite = sprite_new(NULL);
    sprite->x = 100;
    sprite->y = 100;
    sprite->width = 50;
    sprite->height = 50;

    // Point inside
    ASSERT(physics_collides_point(sprite, 125, 125));

    // Point outside
    ASSERT(!physics_collides_point(sprite, 0, 0));
    ASSERT(!physics_collides_point(sprite, 200, 200));

    // Point on edge (should be inside)
    ASSERT(physics_collides_point(sprite, 100, 100));

    teardown_test_env();
}

// ============================================================================
// Collision Detection Tests - Circle
// ============================================================================

TEST(collides_circle_overlapping) {
    setup_test_env();

    ObjSprite* a = sprite_new(NULL);
    a->x = 100;
    a->y = 100;
    a->width = 50;
    a->height = 50;

    ObjSprite* b = sprite_new(NULL);
    b->x = 130;
    b->y = 100;
    b->width = 50;
    b->height = 50;

    ASSERT(physics_collides_circle(a, b));

    teardown_test_env();
}

TEST(collides_circle_not_overlapping) {
    setup_test_env();

    ObjSprite* a = sprite_new(NULL);
    a->x = 0;
    a->y = 0;
    a->width = 50;
    a->height = 50;

    ObjSprite* b = sprite_new(NULL);
    b->x = 200;
    b->y = 200;
    b->width = 50;
    b->height = 50;

    ASSERT(!physics_collides_circle(a, b));

    teardown_test_env();
}

// ============================================================================
// Distance Tests
// ============================================================================

TEST(distance_basic) {
    setup_test_env();

    ObjSprite* a = sprite_new(NULL);
    a->x = 0;
    a->y = 0;
    a->width = 10;
    a->height = 10;

    ObjSprite* b = sprite_new(NULL);
    b->x = 100;
    b->y = 0;
    b->width = 10;
    b->height = 10;

    // Distance between centers (5, 5) and (105, 5) = 100
    ASSERT(FLOAT_EQ(physics_distance(a, b), 100.0));

    teardown_test_env();
}

TEST(distance_diagonal) {
    setup_test_env();

    ObjSprite* a = sprite_new(NULL);
    a->x = 0;
    a->y = 0;
    a->width = 0;
    a->height = 0;

    ObjSprite* b = sprite_new(NULL);
    b->x = 30;
    b->y = 40;
    b->width = 0;
    b->height = 0;

    // 3-4-5 triangle (30-40-50)
    ASSERT(FLOAT_EQ(physics_distance(a, b), 50.0));

    teardown_test_env();
}

// ============================================================================
// Physics Update Tests
// ============================================================================

TEST(physics_update_velocity) {
    setup_test_env();

    ObjSprite* sprite = sprite_new(NULL);
    sprite->x = 100;
    sprite->y = 100;
    sprite->velocity_x = 50;
    sprite->velocity_y = 100;
    sprite->friction = 1.0;  // No friction

    physics_update_sprite(sprite, 1.0);

    ASSERT(FLOAT_EQ(sprite->x, 150.0));
    ASSERT(FLOAT_EQ(sprite->y, 200.0));

    teardown_test_env();
}

TEST(physics_update_acceleration) {
    setup_test_env();

    ObjSprite* sprite = sprite_new(NULL);
    sprite->x = 0;
    sprite->y = 0;
    sprite->velocity_x = 0;
    sprite->velocity_y = 0;
    sprite->acceleration_x = 100;
    sprite->acceleration_y = 50;
    sprite->friction = 1.0;

    physics_update_sprite(sprite, 1.0);

    // After 1 second: velocity should be (100, 50), position should be (100, 50)
    ASSERT(FLOAT_EQ(sprite->velocity_x, 100.0));
    ASSERT(FLOAT_EQ(sprite->velocity_y, 50.0));
    ASSERT(FLOAT_EQ(sprite->x, 100.0));
    ASSERT(FLOAT_EQ(sprite->y, 50.0));

    teardown_test_env();
}

TEST(physics_update_gravity) {
    setup_test_env();

    physics_set_gravity(500.0);

    ObjSprite* sprite = sprite_new(NULL);
    sprite->x = 100;
    sprite->y = 100;
    sprite->velocity_x = 0;
    sprite->velocity_y = 0;
    sprite->gravity_scale = 1.0;
    sprite->friction = 1.0;

    physics_update_sprite(sprite, 1.0);

    // After 1 second with 500 gravity: velocity_y = 500, y = 600
    ASSERT(FLOAT_EQ(sprite->velocity_y, 500.0));
    ASSERT(FLOAT_EQ(sprite->y, 600.0));

    physics_set_gravity(0.0);
    teardown_test_env();
}

TEST(physics_update_gravity_scale) {
    setup_test_env();

    physics_set_gravity(500.0);

    ObjSprite* sprite = sprite_new(NULL);
    sprite->x = 0;
    sprite->y = 0;
    sprite->velocity_x = 0;
    sprite->velocity_y = 0;
    sprite->gravity_scale = 0.5;  // Half gravity
    sprite->friction = 1.0;

    physics_update_sprite(sprite, 1.0);

    // Half gravity = 250
    ASSERT(FLOAT_EQ(sprite->velocity_y, 250.0));

    physics_set_gravity(0.0);
    teardown_test_env();
}

TEST(physics_update_zero_gravity_scale) {
    setup_test_env();

    physics_set_gravity(500.0);

    ObjSprite* sprite = sprite_new(NULL);
    sprite->x = 0;
    sprite->y = 0;
    sprite->velocity_x = 0;
    sprite->velocity_y = 0;
    sprite->gravity_scale = 0.0;  // No gravity effect
    sprite->friction = 1.0;

    physics_update_sprite(sprite, 1.0);

    // No gravity effect
    ASSERT(FLOAT_EQ(sprite->velocity_y, 0.0));
    ASSERT(FLOAT_EQ(sprite->y, 0.0));

    physics_set_gravity(0.0);
    teardown_test_env();
}

// ============================================================================
// Movement Helper Tests
// ============================================================================

TEST(apply_force) {
    setup_test_env();

    ObjSprite* sprite = sprite_new(NULL);
    sprite->acceleration_x = 0;
    sprite->acceleration_y = 0;

    physics_apply_force(sprite, 100, 50);

    ASSERT(FLOAT_EQ(sprite->acceleration_x, 100.0));
    ASSERT(FLOAT_EQ(sprite->acceleration_y, 50.0));

    physics_apply_force(sprite, 20, 30);

    ASSERT(FLOAT_EQ(sprite->acceleration_x, 120.0));
    ASSERT(FLOAT_EQ(sprite->acceleration_y, 80.0));

    teardown_test_env();
}

TEST(look_at_right) {
    setup_test_env();

    ObjSprite* sprite = sprite_new(NULL);
    sprite->x = 0;
    sprite->y = 0;

    physics_look_at(sprite, 100, 0);

    // Looking right should be 0 degrees
    ASSERT(FLOAT_EQ(sprite->rotation, 0.0));

    teardown_test_env();
}

TEST(look_at_down) {
    setup_test_env();

    ObjSprite* sprite = sprite_new(NULL);
    sprite->x = 0;
    sprite->y = 0;

    physics_look_at(sprite, 0, 100);

    // Looking down should be 90 degrees
    ASSERT(FLOAT_EQ(sprite->rotation, 90.0));

    teardown_test_env();
}

TEST(move_toward_not_reached) {
    setup_test_env();

    ObjSprite* sprite = sprite_new(NULL);
    sprite->x = 0;
    sprite->y = 0;

    // Move toward (100, 0) at speed 50 for 1 second
    bool reached = physics_move_toward(sprite, 100, 0, 50, 1.0);

    ASSERT(!reached);
    ASSERT(FLOAT_EQ(sprite->x, 50.0));
    ASSERT(FLOAT_EQ(sprite->y, 0.0));

    teardown_test_env();
}

TEST(move_toward_reached) {
    setup_test_env();

    ObjSprite* sprite = sprite_new(NULL);
    sprite->x = 0;
    sprite->y = 0;

    // Move toward (30, 40) at speed 100 for 1 second
    // Distance is 50, speed is 100, so should reach in 0.5 seconds
    bool reached = physics_move_toward(sprite, 30, 40, 100, 1.0);

    ASSERT(reached);
    ASSERT(FLOAT_EQ(sprite->x, 30.0));
    ASSERT(FLOAT_EQ(sprite->y, 40.0));

    teardown_test_env();
}

// ============================================================================
// Native Function Registration Tests
// ============================================================================

TEST(physics_natives_registered) {
    setup_test_env();

    void* val;
    ASSERT(table_get_cstr(&test_vm.globals, "set_gravity", &val));
    ASSERT(table_get_cstr(&test_vm.globals, "get_gravity", &val));
    ASSERT(table_get_cstr(&test_vm.globals, "collides", &val));
    ASSERT(table_get_cstr(&test_vm.globals, "collides_rect", &val));
    ASSERT(table_get_cstr(&test_vm.globals, "collides_point", &val));
    ASSERT(table_get_cstr(&test_vm.globals, "collides_circle", &val));
    ASSERT(table_get_cstr(&test_vm.globals, "distance", &val));
    ASSERT(table_get_cstr(&test_vm.globals, "apply_force", &val));
    ASSERT(table_get_cstr(&test_vm.globals, "move_toward", &val));
    ASSERT(table_get_cstr(&test_vm.globals, "look_at", &val));
    ASSERT(table_get_cstr(&test_vm.globals, "lerp", &val));
    ASSERT(table_get_cstr(&test_vm.globals, "lerp_angle", &val));

    teardown_test_env();
}

// ============================================================================
// Main
// ============================================================================

int main(void) {
    TEST_SUITE("Math Helpers");
    RUN_TEST(lerp_basic);
    RUN_TEST(lerp_clamps);
    RUN_TEST(lerp_negative);
    RUN_TEST(normalize_angle);
    RUN_TEST(lerp_angle_basic);
    RUN_TEST(lerp_angle_wraparound);

    TEST_SUITE("Gravity");
    RUN_TEST(gravity_default);
    RUN_TEST(gravity_set_get);

    TEST_SUITE("AABB Collision");
    RUN_TEST(collides_overlapping);
    RUN_TEST(collides_not_overlapping);
    RUN_TEST(collides_touching);
    RUN_TEST(collides_rect);
    RUN_TEST(collides_point);

    TEST_SUITE("Circle Collision");
    RUN_TEST(collides_circle_overlapping);
    RUN_TEST(collides_circle_not_overlapping);

    TEST_SUITE("Distance");
    RUN_TEST(distance_basic);
    RUN_TEST(distance_diagonal);

    TEST_SUITE("Physics Update");
    RUN_TEST(physics_update_velocity);
    RUN_TEST(physics_update_acceleration);
    RUN_TEST(physics_update_gravity);
    RUN_TEST(physics_update_gravity_scale);
    RUN_TEST(physics_update_zero_gravity_scale);

    TEST_SUITE("Movement Helpers");
    RUN_TEST(apply_force);
    RUN_TEST(look_at_right);
    RUN_TEST(look_at_down);
    RUN_TEST(move_toward_not_reached);
    RUN_TEST(move_toward_reached);

    TEST_SUITE("Native Registration");
    RUN_TEST(physics_natives_registered);

    TEST_SUMMARY();
}
