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
bool physics_collides_circle(ObjSprite* a, ObjSprite* b)