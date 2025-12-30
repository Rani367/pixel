# Engine API Reference

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

### draw_image_ex(image, x, y, width, height, rotation, flip_x, flip_y)
Draws an image with size, rotation, and flip transformations.

```pixel
draw_image_ex(player_image, 100, 100, 64, 64, 45, false, false)
```

### image_width(image)
Returns the width of an image in pixels.

### image_height(image)
Returns the height of an image in pixels.

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
function on_draw() {
    clear(BLACK)
    draw_sprite(player)
}
```

### Sprite Properties

| Property | Type | Description |
|----------|------|-------------|
| `x` | Number | X position |
| `y` | Number | Y position |
| `width` | Number | Sprite width (0 = use image size) |
| `height` | Number | Sprite height (0 = use image size) |
| `rotation` | Number | Rotation in degrees |
| `scale_x` | Number | Horizontal scale (1.0 = normal) |
| `scale_y` | Number | Vertical scale (1.0 = normal) |
| `origin_x` | Number | X origin for rotation (0-1, 0.5 = center) |
| `origin_y` | Number | Y origin for rotation (0-1, 0.5 = center) |
| `flip_x` | Boolean | Flip horizontally |
| `flip_y` | Boolean | Flip vertically |
| `visible` | Boolean | Whether to draw |

### Sprite Physics Properties

See [Physics API](physics.md) for physics-related sprite properties like `velocity_x`, `velocity_y`, `friction`, and `gravity_scale`.

### Sprite Sheets

For animated sprites using sprite sheets:

| Property | Type | Description |
|----------|------|-------------|
| `frame_x` | Number | X offset in sheet |
| `frame_y` | Number | Y offset in sheet |
| `frame_width` | Number | Frame width |
| `frame_height` | Number | Frame height |

### set_sprite_frame(sprite, frame_index)
Sets the current frame of a sprite sheet by index.

```pixel
set_sprite_frame(player, 0)    // First frame
set_sprite_frame(player, 1)    // Second frame
```

## Animations

### create_animation(image, frame_width, frame_height, frames, frame_time)
Creates an animation from a sprite sheet image.

- `image` - The sprite sheet image
- `frame_width`, `frame_height` - Size of each frame in pixels
- `frames` - List of frame indices to play
- `frame_time` - Duration of each frame in seconds

```pixel
sheet = load_image("player_walk.png")
walk_anim = create_animation(sheet, 32, 32, [0, 1, 2, 3], 0.15)
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
Draws text at the specified position.

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

### camera_x()
Returns the current camera X position.

### camera_y()
Returns the current camera Y position.

### camera_zoom()
Returns the current camera zoom level.

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

### screen_to_world_x(x)
Converts a screen X coordinate to world X coordinate.

### screen_to_world_y(y)
Converts a screen Y coordinate to world Y coordinate.

```pixel
// Get world position of mouse click
world_x = screen_to_world_x(mouse_x())
world_y = screen_to_world_y(mouse_y())
```

### world_to_screen_x(x)
Converts a world X coordinate to screen X coordinate.

### world_to_screen_y(y)
Converts a world Y coordinate to screen Y coordinate.

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
| `emitter_set_lifetime(e, min, max)` | Lifetime range (seconds) |
| `emitter_set_angle(e, min, max)` | Emission angle range (degrees) |
| `emitter_set_gravity(e, gravity)` | Gravity applied to particles |
| `emitter_set_rate(e, rate)` | Emission rate (particles/second) |
| `emitter_set_position(e, x, y)` | Emitter position |
| `emitter_set_active(e, active)` | Enable/disable emission |
| `emitter_count(e)` | Get current particle count |

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
