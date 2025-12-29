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
function on_draw() raws text at the specified position.

```pixel
draw_text("Score: 100", 20, 20, ui_font, WHITE)
```

### text_width(text, font)
Returns the pixel width of the text.

```pixel
width = text_width("Hello", font)
centered_x = window_width() / 2 - width / 2
```

### text_height(text, font)
Returns the pixel height of the text.

## Camera

### camera_set_position(x, y)
Sets the camera position (affects all drawing).

```pixel
camera_set_position(player.x - 400, player.y - 300)
```

### camera_set_zoom(zoom)
Sets the camera zoom level (1.0 = normal).

### camera_follow(sprite, smoothing)
Makes the camera follow a sprite with optional smoothing (0-1).

```pixel
camera_follow(player, 0.1)  // Smooth follow
```

### camera_shake(intensity, duration)
Adds screen shake effect.

```pixel
camera_shake(5, 0.3)  // 5 pixel shake for 0.3 seconds
```

### screen_to_world(x, y)
Converts screen coordinates to world coordinates.

### world_to_screen(x, y)
Converts world coordinates to screen coordinates.

## Particles

### create_emitter(x, y)
Creates a particle emitter at position.

```pixel
emitter = create_emitter(400, 300)
```

### emitter_emit(emitter, count)
Emits particles from the emitter.

```pixel
emitter_emit(explosion, 50)
```

### Emitter Configuration

| Function | Description |
|----------|-------------|
| `emitter_set_color(e, color)` | Particle color |
| `emitter_set_size(e, min, max)` | Size range |
| `emitter_set_speed(e, min, max)` | Speed range |
| `emitter_set_lifetime(e, min, max)` | Lifetime range |
| `emitter_set_direction(e, angle, spread)` | Direction and spread |

### draw_particles(emitter)
Draws all particles from an emitter.

## Scene Management

### load_scene(name)
Loads a new scene. Calls `{name}_on_start()` if it exists.

```pixel
load_scene("menu")     // Calls menu_on_start()
load_scene("level1")   // Calls level1_on_start()
```

### get_scene()
Returns the current scene name.

### Scene Callbacks

Define scene-specific callbacks:

```pixel
function menu_on_start() {
    // Initialize menu
}

function menu_on_update(dt) {
    // Update menu
}

function menu_on_draw() {
    // Draw menu
}
```
{
    clear(BLACK)
    draw_sprite(player)
}
```

### Sprite Properties

| Property | Type | Description |
|----------|------|-------------|
| `x` | Number | X position |
| `y` | Number | Y position |
| `width` | Number | Image width (read-only) |
| `height` | Number | Image height (read-only) |
| `rotation` | Number | Rotation in degrees |
| `scale` | Number | Uniform scale factor |
| `scale_x` | Number | Horizontal scale |
| `scale_y` | Number | Vertical scale |
| `origin_x` | Number | X origin for rotation (0-1) |
| `origin_y` | Number | Y origin for rotation (0-1) |
| `flip_x` | Boolean | Flip horizontally |
| `flip_y` | Boolean | Flip vertically |
| `visible` | Boolean | Whether to draw |

### Sprite Sheets

For animated sprites using sprite sheets:

| Property | Type | Description |
|----------|------|-------------|
| `frame_x` | Number | X offset in sheet |
| `frame_y` | Number | Y offset in sheet |
| `frame_width` | Number | Frame width |
| `frame_height` | Number | Frame height |

### set_sprite_frame(sprite, x, y, width, height)
Sets the sprite sheet region to display.

```pixel
set_sprite_frame(player, 0, 0, 32, 32)    // First frame
set_sprite_frame(player, 32, 0, 32, 32)   // Second frame
```

## Animations

### create_animation(name, frame_count, frame_duration)
Creates a named animation.

```pixel
walk_anim = create_animation("walk", 4, 0.15)
```

### sprite_set_animation(sprite, animation)
Assigns an animation to a sprite.

```pixel
sprite_set_animation(player, walk_anim)
```

### sprite_play(sprite)
Starts playing the sprite's animation.

### sprite_stop(sprite)
Stops the animation.

## Fonts and Text

### default_font(size)
Gets the built-in font at the specified size.

```pixel
title_font = default_font(48)
ui_font = default_font(24)
```

### load_font(path, size)
Loads a custom TrueType font.

```pixel
custom_font = load_font("fonts/arcade.ttf", 32)
```

### draw_text(text, x, y, font, color)
D