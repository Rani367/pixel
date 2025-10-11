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
