ge:
```pixel
angle_rad = angle_deg * DEG_TO_RAD
angle_deg = angle_rad * RAD_TO_DEG
```
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

