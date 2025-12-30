# Physics & Collision API Reference

## Collision Detection

### collides(sprite1, sprite2)
Returns `true` if two sprites overlap (AABB collision).

```pixel
if collides(player, enemy) {
    take_damage()
}
```

### collides_rect(x1, y1, w1, h1, x2, y2, w2, h2)
Returns `true` if two rectangles overlap.

```pixel
if collides_rect(player_x, player_y, 30, 40, coin_x, coin_y, 20, 20) {
    collect_coin()
}
```

### collides_point(x, y, sprite)
Returns `true` if a point is inside a sprite.

```pixel
if collides_point(mouse_x(), mouse_y(), button) {
    button_hovered = true
}
```

### collides_circle(x1, y1, r1, x2, y2, r2)
Returns `true` if two circles overlap.

```pixel
if collides_circle(ball_x, ball_y, 10, player_x, player_y, 25) {
    bounce_ball()
}
```

## Distance

### distance(x1, y1, x2, y2)
Returns the distance between two points.

```pixel
dist = distance(player.x, player.y, enemy.x, enemy.y)
if dist < attack_range {
    can_attack = true
}
```

## Sprite Physics Properties

Sprites have built-in physics properties that are automatically updated each frame.

### Velocity

| Property | Type | Description |
|----------|------|-------------|
| `velocity_x` | Number | Horizontal velocity (pixels/sec) |
| `velocity_y` | Number | Vertical velocity (pixels/sec) |

```pixel
player.velocity_x = 200  // Move right at 200 px/sec
player.velocity_y = -400 // Jump upward
```

### Acceleration

| Property | Type | Description |
|----------|------|-------------|
| `acceleration_x` | Number | Horizontal acceleration |
| `acceleration_y` | Number | Vertical acceleration |

```pixel
// Apply thrust
if key_down(KEY_RIGHT) {
    player.acceleration_x = 500
} else {
    player.acceleration_x = 0
}
```

### Friction

| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `friction` | Number | 0 | Velocity damping (0-1) |

```pixel
player.friction = 0.1  // Slow down gradually
```

### Gravity

| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `gravity_scale` | Number | 1 | Gravity multiplier |
| `grounded` | Boolean | false | Whether sprite is on ground |

```pixel
player.gravity_scale = 1    // Normal gravity
player.gravity_scale = 0    // No gravity (flying)
player.gravity_scale = 0.5  // Half gravity (moon)

if player.grounded {
    can_jump = true
}
```

## Global Gravity

### set_gravity(strength)
Sets the global gravity value (pixels/sec^2). Default is 980.

```pixel
set_gravity(500)   // Low gravity
set_gravity(1500)  // High gravity
set_gravity(0)     // No gravity
```

## Physics Helpers

### apply_force(sprite, force_x, force_y)
Applies a force to a sprite, modifying its velocity.

```pixel
if key_pressed(KEY_SPACE) and player.grounded {
    apply_force(player, 0, -800)  // Jump
    player.grounded = false
}
```

### move_toward(current, target, max_delta)
Moves a value toward a target by at most max_delta.

```pixel
function on_update(dt) {
    // Smoothly move toward target
    player.x = move_toward(player.x, target_x, speed * dt)
    player.y = move_toward(player.y, target_y, speed * dt)
}
```

### look_at(sprite, x, y)
Rotates a sprite to face a point.

```pixel
function on_update(dt) {
    // Face the mouse cursor
    look_at(player, mouse_x(), mouse_y())
}
```

## Math Utilities

### lerp(a, b, t)
Linear interpolation between a and b by factor t (0-1).

```pixel
// Smooth camera follow
camera_x = lerp(camera_x, target_x, 0.1)
camera_y = lerp(camera_y, target_y, 0.1)
```

### lerp_angle(a, b, t)
Interpolates between two angles, handling wraparound.

```pixel
// Smooth rotation toward target
current_angle = lerp_angle(current_angle, target_angle, 0.1)
```

### clamp(value, min, max)
Constrains a value between min and max.

```pixel
player.x = clamp(player.x, 0, window_width() - player.width)
health = clamp(health, 0, max_health)
```

## Common Patterns

### Platformer Physics

```pixel
GRAVITY = 1200
JUMP_FORCE = 500
MOVE_SPEED = 200

function on_update(dt) {
    // Horizontal movement
    if key_down(KEY_LEFT) {
        player.velocity_x = -MOVE_SPEED
    } else if key_down(KEY_RIGHT) {
        player.velocity_x = MOVE_SPEED
    } else {
        player.velocity_x = 0
    }

    // Jumping
    if key_pressed(KEY_SPACE) and player.grounded {
        player.velocity_y = -JUMP_FORCE
        player.grounded = false
    }

    // Apply gravity
    player.velocity_y = player.velocity_y + GRAVITY * dt

    // Move player
    player.x = player.x + player.velocity_x * dt
    player.y = player.y + player.velocity_y * dt

    // Ground collision (simple)
    if player.y > ground_y {
        player.y = ground_y
        player.velocity_y = 0
        player.grounded = true
    }
}
```

### Top-Down Movement

```pixel
function on_update(dt) {
    dx = 0
    dy = 0

    if key_down(KEY_W) { dy = -1 }
    if key_down(KEY_S) { dy = 1 }
    if key_down(KEY_A) { dx = -1 }
    if key_down(KEY_D) { dx = 1 }

    // Normalize diagonal movement
    if dx != 0 and dy != 0 {
        dx = dx * 0.707
        dy = dy * 0.707
    }

    player.x = player.x + dx * speed * dt
    player.y = player.y + dy * speed * dt
}
```

### Bullet/Projectile

```pixel
bullets = []

function spawn_bullet(x, y, angle) {
    bullet = {}
    bullet.x = x
    bullet.y = y
    bullet.vel_x = cos(angle) * BULLET_SPEED
    bullet.vel_y = sin(angle) * BULLET_SPEED
    push(bullets, bullet)
}

function update_bullets(dt) {
    i = len(bullets) - 1
    while i >= 0 {
        b = bullets[i]
        b.x = b.x + b.vel_x * dt
        b.y = b.y + b.vel_y * dt

        // Remove if off screen
        if b.x < 0 or b.x > window_width() or b.y < 0 or b.y > window_height() {
            remove(bullets, i)
        }
        i = i - 1
    }
}
```

### Simple Enemy AI

```pixel
function update_enemy(enemy, dt) {
    // Move toward player
    dx = player.x - enemy.x
    dy = player.y - enemy.y
    dist = sqrt(dx * dx + dy * dy)

    if dist > 0 {
        enemy.x = enemy.x + (dx / dist) * enemy.speed * dt
        enemy.y = enemy.y + (dy / dist) * enemy.speed * dt
    }

    // Face player
    look_at(enemy, player.x, player.y)
}
```

### Bounce Off Walls

```pixel
function on_update(dt) {
    ball.x = ball.x + ball.vel_x * dt
    ball.y = ball.y + ball.vel_y * dt

    // Bounce off sides
    if ball.x <= 0 or ball.x >= window_width() - ball_size {
        ball.vel_x = -ball.vel_x
    }

    // Bounce off top/bottom
    if ball.y <= 0 or ball.y >= window_height() - ball_size {
        ball.vel_y = -ball.vel_y
    }
}
```
