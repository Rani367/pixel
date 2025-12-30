// Physics & Collision System
// Provides collision detection and basic physics for sprites

#ifndef PH_PHYSICS_H
#define PH_PHYSICS_H

#include "core/common.h"
#include "vm/object.h"

// ============================================================================
// Physics World Settings
// ============================================================================

// Set global gravity (applied to all sprites based on their gravity_scale)
void physics_set_gravity(double gravity);

// Get current global gravity
double physics_get_gravity(void);

// ============================================================================
// Physics Update
// ============================================================================

// Update a single sprite's physics (velocity, acceleration, gravity, friction)
// Called automatically each frame before on_update callback
void physics_update_sprite(ObjSprite* sprite, double dt);

// ============================================================================
// Collision Detection - AABB (Axis-Aligned Bounding Box)
// ============================================================================

// Check if two sprites overlap (AABB collision)
bool physics_collides(ObjSprite* a, ObjSprite* b);

// Check if a sprite overlaps with a rectangle
bool physics_collides_rect(ObjSprite* sprite, double x, double y, double w, double h);

// Check if a point is inside a sprite's bounding box
bool physics_collides_point(ObjSprite* sprite, double px, double py);

// ============================================================================
// Collision Detection - Circle
// ============================================================================

// Check if two sprites overlap (circle collision using min dimension as radius)
bool physics_collides_circle(ObjSprite* a, ObjSprite* b);

// ============================================================================
// Distance & Movement Helpers
// ============================================================================

// Get the distance between two sprites' centers
double physics_distance(ObjSprite* a, ObjSprite* b);

// Get the effective width of a sprite (accounting for scale)
double physics_sprite_width(ObjSprite* sprite);

// Get the effective height of a sprite (accounting for scale)
double physics_sprite_height(ObjSprite* sprite);

// Get the center X of a sprite
double physics_sprite_center_x(ObjSprite* sprite);

// Get the center Y of a sprite
double physics_sprite_center_y(ObjSprite* sprite);

// ============================================================================
// Movement Helpers
// ============================================================================

// Move sprite toward a point at a given speed
// Returns true if the sprite reached the destination
bool physics_move_toward(ObjSprite* sprite, double target_x, double target_y,
                         double speed, double dt);

// Rotate sprite to face a point
void physics_look_at(ObjSprite* sprite, double target_x, double target_y);

// Apply a force to a sprite (adds to acceleration)
void physics_apply_force(ObjSprite* sprite, double fx, double fy);

// ============================================================================
// Math Helpers
// ============================================================================

// Linear interpolation between two values
double physics_lerp(double a, double b, double t);

// Angle interpolation (handles wraparound correctly)
double physics_lerp_angle(double a, double b, double t);

// Normalize an angle to 0-360 range
double physics_normalize_angle(double angle);

#endif // PH_PHYSICS_H
