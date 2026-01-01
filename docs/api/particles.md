---
title: "Particle System API Reference"
description: "Create visual effects with Pixel's particle system. Build explosions, fire, smoke, sparkles, and other effects using particle emitters."
keywords: ["Pixel particles", "particle effects", "game effects", "explosion effect", "fire effect"]
---

# Particle System API Reference

Pixel's particle system creates visual effects like explosions, fire, smoke, and sparkles. Particles are lightweight visual elements that spawn, animate, and disappear over time.

## Creating Emitters

### create_emitter(x, y)
Creates a new particle emitter at the specified position.

```pixel
explosion = create_emitter(400, 300)
```

Emitters don't emit particles automatically - you need to configure them and then call `emitter_emit()`.

## Emitting Particles

### emitter_emit(emitter, count)
Emits the specified number of particles from the emitter.

```pixel
// Emit a burst of 50 particles
emitter_emit(explosion, 50)

// Emit just a few particles
emitter_emit(sparkle, 5)
```

### draw_particles(emitter)
Draws all active particles from an emitter. Call this in `on_draw()`.

```pixel
function on_draw() {
    clear(BLACK)
    draw_particles(explosion)
    draw_particles(sparkle)
}
```

## Emitter Configuration

### emitter_set_position(emitter, x, y)
Sets the emitter's position.

```pixel
// Move emitter to player position
emitter_set_position(trail, player.x, player.y)
```

### emitter_set_color(emitter, color)
Sets the color of emitted particles.

```pixel
emitter_set_color(fire, rgb(255, 100, 0))
emitter_set_color(smoke, rgba(100, 100, 100, 128))
```

### emitter_set_size(emitter, min, max)
Sets the size range for particles.

```pixel
// Small particles (2-4 pixels)
emitter_set_size(dust, 2, 4)

// Larger particles (5-10 pixels)
emitter_set_size(explosion, 5, 10)
```

### emitter_set_speed(emitter, min, max)
Sets the speed range for particles (pixels per second).

```pixel
// Slow drifting particles
emitter_set_speed(dust, 10, 30)

// Fast explosion particles
emitter_set_speed(explosion, 100, 300)
```

### emitter_set_lifetime(emitter, min, max)
Sets how long particles live (in seconds).

```pixel
// Short-lived spark
emitter_set_lifetime(spark, 0.1, 0.3)

// Longer smoke trail
emitter_set_lifetime(smoke, 1.0, 2.0)
```

### emitter_set_angle(emitter, min, max)
Sets the emission angle range (in degrees). 0 = right, 90 = down, 180 = left, 270 = up.

```pixel
// Emit in all directions (360 degrees)
emitter_set_angle(explosion, 0, 360)

// Emit upward only (with some spread)
emitter_set_angle(fire, 250, 290)

// Emit to the right
emitter_set_angle(trail, -20, 20)
```

### emitter_set_gravity(emitter, gravity)
Sets gravity applied to particles (pixels per second squared).

```pixel
// No gravity (floats)
emitter_set_gravity(sparkle, 0)

// Light gravity (drifts down)
emitter_set_gravity(snow, 50)

// Normal gravity (falls)
emitter_set_gravity(debris, 500)

// Negative gravity (rises)
emitter_set_gravity(smoke, -100)
```

### emitter_set_rate(emitter, rate)
Sets continuous emission rate (particles per second). Use with `emitter_set_active()`.

```pixel
emitter_set_rate(trail, 30)  // 30 particles per second
```

### emitter_set_active(emitter, active)
Enables or disables continuous emission.

```pixel
// Start continuous emission
emitter_set_active(trail, true)

// Stop continuous emission
emitter_set_active(trail, false)
```

## Emitter Status

### emitter_count(emitter)
Returns the number of active particles.

```pixel
count = emitter_count(explosion)
if count == 0 {
    // All particles have faded
}
```

## Effect Recipes

### Explosion

```pixel
explosion = none

function create_explosion(x, y) {
    explosion = create_emitter(x, y)
    emitter_set_color(explosion, rgb(255, 200, 50))
    emitter_set_size(explosion, 3, 8)
    emitter_set_speed(explosion, 100, 300)
    emitter_set_lifetime(explosion, 0.3, 0.6)
    emitter_set_angle(explosion, 0, 360)
    emitter_set_gravity(explosion, 200)
    emitter_emit(explosion, 50)
}
```

### Fire

```pixel
fire = none

function on_start() {
    create_window(800, 600, "Fire Demo")

    fire = create_emitter(400, 500)
    emitter_set_color(fire, rgb(255, 100, 0))
    emitter_set_size(fire, 4, 8)
    emitter_set_speed(fire, 50, 100)
    emitter_set_lifetime(fire, 0.5, 1.0)
    emitter_set_angle(fire, 250, 290)  // Upward
    emitter_set_gravity(fire, -50)      // Rise up
    emitter_set_rate(fire, 20)
    emitter_set_active(fire, true)
}
```

### Smoke Trail

```pixel
trail = none

function on_start() {
    trail = create_emitter(0, 0)
    emitter_set_color(trail, rgba(150, 150, 150, 100))
    emitter_set_size(trail, 3, 6)
    emitter_set_speed(trail, 20, 50)
    emitter_set_lifetime(trail, 0.5, 1.5)
    emitter_set_angle(trail, 160, 200)  // Behind (left)
    emitter_set_gravity(trail, -30)
    emitter_set_rate(trail, 15)
}

function on_update(dt) {
    // Follow the player
    emitter_set_position(trail, player.x, player.y)

    // Only emit when moving
    if key_down(KEY_RIGHT) {
        emitter_set_active(trail, true)
    } else {
        emitter_set_active(trail, false)
    }
}
```

### Sparkle/Pickup Effect

```pixel
function create_sparkle(x, y) {
    sparkle = create_emitter(x, y)
    emitter_set_color(sparkle, rgb(255, 255, 100))
    emitter_set_size(sparkle, 2, 4)
    emitter_set_speed(sparkle, 50, 150)
    emitter_set_lifetime(sparkle, 0.2, 0.5)
    emitter_set_angle(sparkle, 0, 360)
    emitter_set_gravity(sparkle, 0)
    emitter_emit(sparkle, 20)
    return sparkle
}
```

### Snow

```pixel
snow = none

function on_start() {
    create_window(800, 600, "Snow Demo")

    snow = create_emitter(400, -10)
    emitter_set_color(snow, WHITE)
    emitter_set_size(snow, 2, 4)
    emitter_set_speed(snow, 20, 60)
    emitter_set_lifetime(snow, 5.0, 8.0)
    emitter_set_angle(snow, 80, 100)   // Mostly down
    emitter_set_gravity(snow, 20)
    emitter_set_rate(snow, 10)
    emitter_set_active(snow, true)
}
```

## Complete Example

```pixel
explosion = none
trail = none
player_x = 400
player_y = 300

function on_start() {
    create_window(800, 600, "Particle Demo")

    // Create explosion emitter (burst)
    explosion = create_emitter(0, 0)
    emitter_set_color(explosion, rgb(255, 150, 50))
    emitter_set_size(explosion, 4, 10)
    emitter_set_speed(explosion, 100, 250)
    emitter_set_lifetime(explosion, 0.3, 0.8)
    emitter_set_angle(explosion, 0, 360)
    emitter_set_gravity(explosion, 300)

    // Create trail emitter (continuous)
    trail = create_emitter(player_x, player_y)
    emitter_set_color(trail, rgba(100, 150, 255, 150))
    emitter_set_size(trail, 2, 5)
    emitter_set_speed(trail, 30, 80)
    emitter_set_lifetime(trail, 0.3, 0.6)
    emitter_set_angle(trail, 0, 360)
    emitter_set_gravity(trail, -50)
    emitter_set_rate(trail, 20)
}

function on_update(dt) {
    // Move player
    if key_down(KEY_LEFT)  { player_x -= 200 * dt }
    if key_down(KEY_RIGHT) { player_x += 200 * dt }
    if key_down(KEY_UP)    { player_y -= 200 * dt }
    if key_down(KEY_DOWN)  { player_y += 200 * dt }

    // Update trail position
    emitter_set_position(trail, player_x, player_y)

    // Trail active only when moving
    moving = key_down(KEY_LEFT) or key_down(KEY_RIGHT) or
             key_down(KEY_UP) or key_down(KEY_DOWN)
    emitter_set_active(trail, moving)

    // Explode on click
    if mouse_pressed(MOUSE_LEFT) {
        emitter_set_position(explosion, mouse_x(), mouse_y())
        emitter_emit(explosion, 40)
    }
}

function on_draw() {
    clear(rgb(20, 20, 40))
    draw_particles(trail)
    draw_particles(explosion)
    draw_circle(player_x, player_y, 15, CYAN)
}
```

## See Also

- [Engine API](/pixel/docs/api/engine) - Drawing and sprites
- [Camera API](/pixel/docs/api/camera) - Camera effects like shake
