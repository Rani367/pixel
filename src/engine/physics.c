// Physics & Collision System Implementation

#include "engine/physics.h"
#include <math.h>

// ============================================================================
// Global Physics State
// ============================================================================

static double global_gravity = 0.0;  // Default: no gravity

// ============================================================================
// Physics World Settings
// ============================================================================

void physics_set_gravity(double gravity) {
    global_gravity = gravity;
}

double physics_get_gravity(void) {
    return global_gravity;
}

// ============================================================================
// Sprite Dimension Helpers
// ============================================================================

double physics_sprite_width(ObjSprite* sprite) {
    double base_width = sprite->width > 0 ? sprite->width :
                        (sprite->frame_width > 0 ? sprite->frame_width :
                         (sprite->image ? sprite->image->width : 0));
    return base_width * sprite->scale_x;
}

double physics_sprite_height(ObjSprite* sprite) {
    double base_height = sprite->height > 0 ? sprite->height :
                         (sprite->frame_height > 0 ? sprite->frame_height :
                          (sprite->image ? sprite->image->height : 0));
    return base_height * sprite->scale_y;
}

double physics_sprite_center_x(ObjSprite* sprite) {
    double width = physics_sprite_width(sprite);
    return sprite->x + width * (0.5 - sprite->origin_x);
}

double physics_sprite_center_y(ObjSprite* sprite) {
    double height = physics_sprite_height(sprite);
    return sprite->y + height * (0.5 - sprite->origin_y);
}

// ============================================================================
// Physics Update
// ============================================================================

void physics_update_sprite(ObjSprite* sprite, double dt) {
    // Apply gravity to acceleration (Y axis, positive = down)
    double gravity_force = global_gravity * sprite->gravity_scale;

    // Update velocity from acceleration (including gravity)
    sprite->velocity_x += sprite->acceleration_x * dt;
    sprite->velocity_y += (sprite->acceleration_y + gravity_force) * dt;

    // Apply friction to velocity
    if (sprite->friction < 1.0) {
        // Friction is applied as a multiplier each frame
        // friction = 1.0 means no friction, 0.0 means instant stop
        double friction_factor = pow(sprite->friction, dt * 60.0);
        sprite->velocity_x *= friction_factor;
        sprite->velocity_y *= friction_factor;
    }

    // Update position from velocity
    sprite->x += sprite->velocity_x * dt;
    sprite->y += sprite->velocity_y * dt;
}

// ============================================================================
// Collision Detection - AABB
// ============================================================================

bool physics_collides(ObjSprite* a, ObjSprite* b) {
    if (!a || !b) return false;

    double a_width = physics_sprite_width(a);
    double a_height = physics_sprite_height(a);
    double b_width = physics_sprite_width(b);
    double b_height = physics_sprite_height(b);

    // Calculate top-left corners accounting for origin
    double a_left = a->x - a_width * a->origin_x;
    double a_top = a->y - a_height * a->origin_y;
    double b_left = b->x - b_width * b->origin_x;
    double b_top = b->y - b_height * b->origin_y;

    // AABB overlap test
    return a_left < b_left + b_width &&
           a_left + a_width > b_left &&
           a_top < b_top + b_height &&
           a_top + a_height > b_top;
}

bool physics_collides_rect(ObjSprite* sprite, double x, double y, double w, double h) {
    if (!sprite) return false;

    double s_width = physics_sprite_width(sprite);
    double s_height = physics_sprite_height(sprite);

    // Calculate sprite's top-left corner accounting for origin
    double s_left = sprite->x - s_width * sprite->origin_x;
    double s_top = sprite->y - s_height * sprite->origin_y;

    // AABB overlap test
    return s_left < x + w &&
           s_left + s_width > x &&
           s_top < y + h &&
           s_top + s_height > y;
}

bool physics_collides_point(ObjSprite* sprite, double px, double py) {
    if (!sprite) return false;

    double s_width = physics_sprite_width(sprite);
    double s_height = physics_sprite_height(sprite);

    // Calculate sprite's top-left corner accounting for origin
    double s_left = sprite->x - s_width * sprite->origin_x;
    double s_top = sprite->y - s_height * sprite->origin_y;

    return px >= s_left && px < s_left + s_width &&
           py >= s_top && py < s_top + s_height;
}

// ============================================================================
// Collision Detection - Circle
// ============================================================================

bool physics_collides_circle(ObjSprite* a, ObjSprite* b) {
    if (!a || !b) return false;

    // Use center points
    double ax = physics_sprite_center_x(a);
    double ay = physics_sprite_center_y(a);
    double bx = physics_sprite_center_x(b);
    double by = physics_sprite_center_y(b);

    // Use half of the minimum dimension as radius
    double a_width = physics_sprite_width(a);
    double a_height = physics_sprite_height(a);
    double b_width = physics_sprite_width(b);
    double b_height = physics_sprite_height(b);

    double radius_a = (a_width < a_height ? a_width : a_height) * 0.5;
    double radius_b = (b_width < b_height ? b_width : b_height) * 0.5;

    // Distance squared between centers
    double dx = bx - ax;
    double dy = by - ay;
    double dist_sq = dx * dx + dy * dy;

    // Sum of radii squared
    double radii_sum = radius_a + radius_b;

    return dist_sq < radii_sum * radii_sum;
}

// ============================================================================
// Distance & Movement Helpers
// ============================================================================

double physics_distance(ObjSprite* a, ObjSprite* b) {
    if (!a || !b) return 0.0;

    double ax = physics_sprite_center_x(a);
    double ay = physics_sprite_center_y(a);
    double bx = physics_sprite_center_x(b);
    double by = physics_sprite_center_y(b);

    double dx = bx - ax;
    double dy = by - ay;

    return sqrt(dx * dx + dy * dy);
}

bool physics_move_toward(ObjSprite* sprite, double target_x, double target_y,
                         double speed, double dt) {
    if (!sprite) return false;

    double dx = target_x - sprite->x;
    double dy = target_y - sprite->y;
    double dist = sqrt(dx * dx + dy * dy);

    if (dist < 0.001) {
        // Already at target
        sprite->x = target_x;
        sprite->y = target_y;
        return true;
    }

    double move_dist = speed * dt;

    if (move_dist >= dist) {
        // Would overshoot, snap to target
        sprite->x = target_x;
        sprite->y = target_y;
        return true;
    }

    // Move toward target
    double ratio = move_dist / dist;
    sprite->x += dx * ratio;
    sprite->y += dy * ratio;
    return false;
}

void physics_look_at(ObjSprite* sprite, double target_x, double target_y) {
    if (!sprite) return;

    double dx = target_x - sprite->x;
    double dy = target_y - sprite->y;

    // atan2 returns radians, convert to degrees
    // atan2(dy, dx) gives angle from positive X axis
    // For games, 0 degrees typically points right, 90 points down
    sprite->rotation = atan2(dy, dx) * (180.0 / M_PI);
}

void physics_apply_force(ObjSprite* sprite, double fx, double fy) {
    if (!sprite) return;

    sprite->acceleration_x += fx;
    sprite->acceleration_y += fy;
}

// ============================================================================
// Math Helpers
// ============================================================================

double physics_lerp(double a, double b, double t) {
    // Clamp t to [0, 1]
    if (t < 0.0) t = 0.0;
    if (t > 1.0) t = 1.0;
    return a + (b - a) * t;
}

double physics_normalize_angle(double angle) {
    // Normalize to 0-360 range
    angle = fmod(angle, 360.0);
    if (angle < 0.0) angle += 360.0;
    return angle;
}

double physics_lerp_angle(double a, double b, double t) {
    // Normalize both angles
    a = physics_normalize_angle(a);
    b = physics_normalize_angle(b);

    // Find the shortest path
    double diff = b - a;

    if (diff > 180.0) {
        diff -= 360.0;
    } else if (diff < -180.0) {
        diff += 360.0;
    }

    double result = a + diff * t;
    return physics_normalize_angle(result);
}
