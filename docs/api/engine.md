---
title: "Engine API Reference"
description: "Pixel game engine API: window management, drawing primitives, sprites, images, fonts, and text rendering."
keywords: ["Pixel engine", "sprite API", "drawing functions", "game graphics", "text rendering"]
---

# Engine API Reference

This page covers window management, drawing, sprites, and text. For other engine features, see:

- [Animation API](/pixel/docs/api/animation) - Sprite sheet animations
- [Camera API](/pixel/docs/api/camera) - Camera position, zoom, shake, and follow
- [Particles API](/pixel/docs/api/particles) - Particle effects and emitters
- [Scenes API](/pixel/docs/api/scenes) - Scene management and transitions

## Window Management

### create_window(width, height, title)
Creates the game window. Must be called in `on_start()`.

```pixel
function on_start() {
    create_window(800, 600, "My Game")
}
```

### set_title(title)
Changes the window title.

```pixel
set_title("My Game - Score: " + to_string(score))
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

Note: The `dt` parameter passed to `on_update(dt)` is the same as `delta_time()`.

### game_time()
Returns total time since game start in seconds.

```pixel
// Animate based on time
offset = sin(game_time() * 2) * 10
```

## Colors

### rgb(r, g, b)
Creates a color from red, green, blue values (0-255).

```pixel
sky_blue = rgb(135, 206, 235)
dark_gray = rgb(50, 50, 50)
```

### rgba(r, g, b, a)
Creates a color with alpha transparency (0-255).

```pixel
semi_transparent = rgba(255, 0, 0, 128)  // 50% transparent red
invisible = rgba(0, 0, 0, 0)             // Fully transparent
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
| `PURPLE` | 128, 0, 128 |
| `GRAY` | 128, 128, 128 |
| `GREY` | 128, 128, 128 |

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

### draw_rect_outline(x, y, width, height, color)
Draws a rectangle outline (not filled).

```pixel
draw_rect_outline(100, 100, 50, 30, WHITE)
```

### draw_circle(x, y, radius, color)
Draws a filled circle centered at (x, y).

```pixel
draw_circle(400, 300, 25, YELLOW)
```

### draw_circle_outline(x, y, radius, color)
Draws a circle outline (not filled).

```pixel
draw_circle_outline(400, 300, 25, WHITE)
```

### draw_line(x1, y1, x2, y2, color)
Draws a line between two points.

```pixel
draw_line(0, 0, 800, 600, WHITE)
```

## Images

### load_image(path)
Loads an image from file. Returns an image object. Supports PNG, JPG, BMP.

```pixel
player_image = load_image("assets/player.png")
background = load_image("assets/bg.jpg")
```

### draw_image(image, x, y)
Draws an image at the specified position (top-left corner).

```pixel
draw_image(player_image, 100, 100)
```

### draw_image_ex(image, x, y, width, height, rotation, flip_x, flip_y)
Draws an image with transformations.

| Parameter | Description |
|-----------|-------------|
| `x`, `y` | Position (top-left corner) |
| `width`, `height` | Size in pixels (0 = original size) |
| `rotation` | Rotation in degrees |
| `flip_x` | Flip horizontally (true/false) |
| `flip_y` | Flip vertically (true/false) |

```pixel
// Draw at double size, rotated 45 degrees
draw_image_ex(player_image, 100, 100, 64, 64, 45, false, false)

// Draw flipped horizontally
draw_image_ex(player_image, 100, 100, 0, 0, 0, true, false)
```

### image_width(image)
Returns the width of an image in pixels.

### image_height(image)
Returns the height of an image in pixels.

```pixel
img = load_image("player.png")
println("Size: " + to_string(image_width(img)) + "x" + to_string(image_height(img)))
```

## Sprites

Sprites are game objects with position, rotation, and other properties. They're ideal for game entities like players, enemies, and items.

### create_sprite(image)
Creates a sprite from an image. Pass `none` for a sprite without an image (useful for collision boxes).

```pixel
player = create_sprite(load_image("player.png"))
player.x = 400
player.y = 300

// Collision-only sprite (no image)
hitbox = create_sprite(none)
hitbox.width = 50
hitbox.height = 50
```

### draw_sprite(sprite)
Draws a sprite at its current position with all transformations.

```pixel
function on_draw() {
    clear(BLACK)
    draw_sprite(player)
    draw_sprite(enemy)
}
```

### Sprite Properties

| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `x` | Number | 0 | X position |
| `y` | Number | 0 | Y position |
| `width` | Number | 0 | Sprite width (0 = use image size) |
| `height` | Number | 0 | Sprite height (0 = use image size) |
| `rotation` | Number | 0 | Rotation in degrees |
| `scale_x` | Number | 1 | Horizontal scale (1.0 = normal) |
| `scale_y` | Number | 1 | Vertical scale (1.0 = normal) |
| `origin_x` | Number | 0.5 | X origin for rotation (0-1, 0.5 = center) |
| `origin_y` | Number | 0.5 | Y origin for rotation (0-1, 0.5 = center) |
| `flip_x` | Boolean | false | Flip horizontally |
| `flip_y` | Boolean | false | Flip vertically |
| `visible` | Boolean | true | Whether to draw |

```pixel
player = create_sprite(load_image("player.png"))
player.x = 400
player.y = 300
player.rotation = 45           // Rotate 45 degrees
player.scale_x = 2             // Double width
player.flip_x = true           // Face left
```

### Sprite Physics Properties

For physics-related properties like `velocity_x`, `velocity_y`, `friction`, and `gravity_scale`, see the [Physics API](/pixel/docs/api/physics).

### Sprite Sheet Properties

For animated sprites using sprite sheets, you can set these properties directly:

| Property | Type | Description |
|----------|------|-------------|
| `frame_x` | Number | X offset in sheet (pixels) |
| `frame_y` | Number | Y offset in sheet (pixels) |
| `frame_width` | Number | Frame width (pixels) |
| `frame_height` | Number | Frame height (pixels) |

For animation playback, see the [Animation API](/pixel/docs/api/animation).

### set_sprite_frame(sprite, frame_index)
Sets the current frame of a sprite sheet by index.

```pixel
set_sprite_frame(player, 0)    // First frame
set_sprite_frame(player, 1)    // Second frame
```

## Fonts and Text

### default_font(size)
Gets the built-in font at the specified size.

```pixel
title_font = default_font(48)
ui_font = default_font(24)
small_font = default_font(16)
```

### load_font(path, size)
Loads a custom TrueType font (.ttf file).

```pixel
custom_font = load_font("fonts/arcade.ttf", 32)
```

### draw_text(text, x, y, font, color)
Draws text at the specified position.

```pixel
draw_text("Score: 100", 20, 20, ui_font, WHITE)
draw_text("GAME OVER", 300, 280, title_font, RED)
```

### text_width(text, font)
Returns the pixel width of the text. Useful for centering.

```pixel
title = "Welcome!"
width = text_width(title, title_font)
x = window_width() / 2 - width / 2
draw_text(title, x, 200, title_font, WHITE)
```

### text_height(text, font)
Returns the pixel height of the text.

## See Also

- [Animation API](/pixel/docs/api/animation) - Sprite sheet animations
- [Camera API](/pixel/docs/api/camera) - Camera control and screen shake
- [Particles API](/pixel/docs/api/particles) - Particle effects
- [Scenes API](/pixel/docs/api/scenes) - Scene management
- [Input API](/pixel/docs/api/input) - Keyboard and mouse
- [Audio API](/pixel/docs/api/audio) - Sound and music
- [Physics API](/pixel/docs/api/physics) - Collision and movement
