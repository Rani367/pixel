# Engine API Reference

## Window Management

### create_window(width, height, title)
Creates the game window. Must be called in `on_start()`.

```pixel
function on_start() {
    create_window(800, 600, "My Game")
}
```

### window_width()
Returns the window width in pixels.

### window_height()
Returns the window height in pixels.

## Timing

### delta_time()
Returns time since last frame in seconds. Use for smooth movement.

```pixel
function on_update(dt) {
    x = x + speed * dt  // Framerate-independent
}
```

### game_time()
Returns total time since game start in seconds.

## Colors

### rgb(r, g, b)
Creates a color from red, green, blue values (0-255).

```pixel
sky_blue = rgb(135, 206, 235)
```

### rgba(r, g, b, a)
Creates a color with alpha transparency (0-255).

```pixel
semi_transparent = rgba(255, 0, 0, 128)
```

### Color Constants

| Constant | RGB Value |
|----------|-----------|
| `WHITE` | 255, 255, 255 |
| `BLACK` | 0, 0, 0 |
| `RED` | 255, 0, 0 |
| `GREEN` | 0, 255, 0 |
| `BLUE` | 0, 0, 255 |
| `YELLOW` | 255, 255, 0 |
| `CYAN` | 0, 255, 255 |
| `MAGENTA` | 255, 0, 255 |
| `ORANGE` | 255, 165, 0 |
| `GRAY` | 128, 128, 128 |

## Drawing Primitives

### clear(color)
Clears the screen with the specified color. Call at the start of `on_draw()`.

```pixel
function on_draw() {
    clear(rgb(20, 20, 40))
    // ... draw everything else
}
```

### draw_rect(x, y, width, height, color)
Draws a filled rectangle.

```pixel
draw_rect(100, 100, 50, 30, RED)
```

### draw_circle(x, y, radius, color)
Draws a filled circle centered at (x, y).

```pixel
draw_circle(400, 300, 25, YELLOW)
```

### draw_line(x1, y1, x2, y2, color)
Draws a line between two points.

```pixel
draw_line(0, 0, 800, 600, WHITE)
```

## Images

### load_image(path)
Loads an image from file. Returns an image object.

```pixel
player_image = load_image("assets/player.png")
```

### draw_image(image, x, y)
Draws an image at the specified position.

```pixel
draw_image(player_image, 100, 100)
```

### draw_image_ex(image, x, y, rotation, scale_x, scale_y)
Draws an image with rotation and scaling.

```pixel
draw_image_ex(player_image, 100, 100, 45, 2.0, 2.0)
```

## Sprites

Sprites are game objects with position, rotation, and other properties.

### create_sprite(image)
Creates a sprite from an image.

```pixel
player = create_sprite(load_image("player.png"))
player.x = 400
player.y = 300
```

### draw_sprite(sprite)
Draws a sprite at its current position with all transformations.

```pixel
function on_draw() 