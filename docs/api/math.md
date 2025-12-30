# Math API Reference

## Basic Math

### abs(n)
Returns the absolute value.

```pixel
distance = abs(x2 - x1)
```

### floor(n)
Rounds down to nearest integer.

```pixel
tile_x = floor(player.x / tile_size)
```

### ceil(n)
Rounds up to nearest integer.

```pixel
pages = ceil(items / per_page)
```

### round(n)
Rounds to nearest integer.

```pixel
display_score = round(score)
```

### min(a, b)
Returns the smaller value.

```pixel
health = min(health + heal, max_health)
```

### max(a, b)
Returns the larger value.

```pixel
damage = max(attack - defense, 1)
```

### clamp(value, min, max)
Constrains a value between min and max.

```pixel
volume = clamp(volume, 0, 1)
position = clamp(x, 0, window_width())
```

## Powers and Roots

### sqrt(n)
Returns the square root.

```pixel
distance = sqrt(dx * dx + dy * dy)
```

### pow(base, exponent)
Returns base raised to exponent.

```pixel
area = pow(radius, 2) * 3.14159
damage = pow(2, level)  // Doubles each level
```

## Trigonometry

All angles are in **radians**. To convert:
- Degrees to radians: `angle * 3.14159 / 180`
- Radians to degrees: `angle * 180 / 3.14159`

### sin(angle)
Returns the sine of an angle.

```pixel
y_offset = sin(time * 2) * 10  // Bobbing motion
```

### cos(angle)
Returns the cosine of an angle.

```pixel
x_offset = cos(time * 2) * 10  // Circular motion
```

### tan(angle)
Returns the tangent of an angle.

### atan2(y, x)
Returns the angle in radians from origin to point (x, y).

```pixel
// Get angle from player to mouse
angle = atan2(mouse_y() - player.y, mouse_x() - player.x)

// Convert to degrees for sprite rotation
player.rotation = angle * 180 / 3.14159
```

## Random Numbers

### random()
Returns a random number between 0.0 (inclusive) and 1.0 (exclusive).

```pixel
if random() < 0.1 {
    spawn_powerup()  // 10% chance
}
```

### random_range(min, max)
Returns a random float between min and max.

```pixel
spawn_x = random_range(50, window_width() - 50)
speed = random_range(100, 200)
```

### random_int(min, max)
Returns a random integer between min and max (inclusive).

```pixel
dice = random_int(1, 6)
enemy_type = random_int(0, 2)
```

## Interpolation

### lerp(a, b, t)
Linear interpolation: returns value between a and b based on t (0-1).

```pixel
// Smooth camera follow
camera.x = lerp(camera.x, target.x, 0.1)

// Fade color
alpha = lerp(0, 255, fade_progress)
```

### lerp_angle(a, b, t)
Interpolates between angles, handling wraparound at 360/0.

```pixel
// Smooth rotation
current_angle = lerp_angle(current_angle, target_angle, 0.1)
```

## Vector Operations

### vec2(x, y)
Creates a 2D vector.

```pixel
position = vec2(100, 200)
velocity = vec2(50, -100)
```

### Vec2 Properties

```pixel
v = vec2(3, 4)
print(v.x)  // 3
print(v.y)  // 4
```

### Vec2 Operations

```pixel
a = vec2(1, 2)
b = vec2(3, 4)

// Addition
c = vec2(a.x + b.x, a.y + b.y)

// Scalar multiplication
scaled = vec2(a.x * 2, a.y * 2)

// Magnitude (length)
length = sqrt(a.x * a.x + a.y * a.y)

// Normalize (unit vector)
if length > 0 {
    normalized = vec2(a.x / length, a.y / length)
}
```

## Common Patterns

### Calculate Distance

```pixel
function get_distance(x1, y1, x2, y2) {
    dx = x2 - x1
    dy = y2 - y1
    return sqrt(dx * dx + dy * dy)
}
```

### Normalize Direction

```pixel
function normalize(dx, dy) {
    length = sqrt(dx * dx + dy * dy)
    if length > 0 {
        return vec2(dx / length, dy / length)
    }
    return vec2(0, 0)
}
```

### Move Toward Target

```pixel
function move_toward_point(x, y, target_x, target_y, speed, dt) {
    dx = target_x - x
    dy = target_y - y
    dist = sqrt(dx * dx + dy * dy)

    if dist < speed * dt {
        return vec2(target_x, target_y)
    }

    return vec2(
        x + (dx / dist) * speed * dt,
        y + (dy / dist) * speed * dt
    )
}
```

### Circular Motion

```pixel
function on_update(dt) {
    angle = angle + rotation_speed * dt
    orbit_x = center_x + cos(angle) * radius
    orbit_y = center_y + sin(angle) * radius
}
```

### Sine Wave Animation

```pixel
function on_update(dt) {
    // Smooth bobbing motion
    bob_offset = sin(game_time() * 3) * 10
}

function on_draw() {
    draw_sprite_at(sprite, x, y + bob_offset)
}
```

### Random Point in Circle

```pixel
function random_in_circle(cx, cy, radius) {
    angle = random() * 6.283  // 2 * PI
    r = sqrt(random()) * radius  // sqrt for uniform distribution
    return vec2(cx + cos(angle) * r, cy + sin(angle) * r)
}
```

### Angle Between Points

```pixel
function angle_between(x1, y1, x2, y2) {
    return atan2(y2 - y1, x2 - x1)
}

// Usage
angle = angle_between(player.x, player.y, enemy.x, enemy.y)
```

### Rotate Point Around Origin

```pixel
function rotate_point(x, y, angle) {
    cos_a = cos(angle)
    sin_a = sin(angle)
    return vec2(
        x * cos_a - y * sin_a,
        x * sin_a + y * cos_a
    )
}
```

## Constants

Common mathematical constants you may need:

```pixel
PI = 3.14159265359
TWO_PI = 6.28318530718
HALF_PI = 1.57079632679
DEG_TO_RAD = 0.01745329251
RAD_TO_DEG = 57.2957795131
```

Usage:
```pixel
angle_rad = angle_deg * DEG_TO_RAD
angle_deg = angle_rad * RAD_TO_DEG
```
