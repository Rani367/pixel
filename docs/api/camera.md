---
title: "Camera API Reference"
description: "Control the game camera in Pixel. Implement smooth scrolling, camera follow, screen shake, zoom, and coordinate conversion for side-scrollers and top-down games."
keywords: ["Pixel camera", "game camera", "screen shake", "camera follow", "zoom", "scrolling"]
---

# Camera API Reference

The camera controls which part of the game world is visible on screen. Use it for scrolling games, zoom effects, and screen shake.

## Camera Position

### camera_x()
Returns the current camera X position.

```pixel
x = camera_x()
```

### camera_y()
Returns the current camera Y position.

```pixel
y = camera_y()
```

### camera_set_position(x, y)
Sets the camera position. All drawing is offset by this position.

```pixel
// Center camera on player (assuming 800x600 window)
camera_set_position(player.x - 400, player.y - 300)
```

## Camera Zoom

### camera_zoom()
Returns the current zoom level (1.0 = normal).

```pixel
zoom = camera_zoom()
```

### camera_set_zoom(zoom)
Sets the camera zoom level.

| Zoom Value | Effect |
|------------|--------|
| `< 1.0` | Zoom out (see more) |
| `1.0` | Normal |
| `> 1.0` | Zoom in (see less) |

```pixel
camera_set_zoom(2.0)   // 2x zoom in
camera_set_zoom(0.5)   // 2x zoom out
camera_set_zoom(1.0)   // Normal
```

## Camera Follow

### camera_follow(sprite, smoothing)
Makes the camera follow a sprite with optional smoothing.

| Parameter | Type | Description |
|-----------|------|-------------|
| `sprite` | Sprite | The sprite to follow |
| `smoothing` | Number | Smoothing factor (0-1). 0 = instant, 1 = very slow |

```pixel
// Instant follow (no smoothing)
camera_follow(player, 0)

// Smooth follow (recommended)
camera_follow(player, 0.1)

// Very smooth/slow follow
camera_follow(player, 0.02)
```

Typical values:
- `0.05` to `0.15` - Good for platformers and action games
- `0.02` to `0.05` - Good for slower-paced games

## Screen Shake

### camera_shake(intensity, duration)
Adds a screen shake effect.

| Parameter | Type | Description |
|-----------|------|-------------|
| `intensity` | Number | Shake amount in pixels |
| `duration` | Number | Duration in seconds |

```pixel
// Small shake for footsteps
camera_shake(2, 0.1)

// Medium shake for explosions
camera_shake(5, 0.3)

// Large shake for big impacts
camera_shake(10, 0.5)
```

## Coordinate Conversion

The camera creates a distinction between **screen coordinates** (where things appear on the display) and **world coordinates** (where things exist in the game world).

### screen_to_world_x(x)
Converts a screen X coordinate to world X coordinate.

### screen_to_world_y(y)
Converts a screen Y coordinate to world Y coordinate.

```pixel
// Get world position of mouse click
function on_mouse_click(button, x, y) {
    world_x = screen_to_world_x(x)
    world_y = screen_to_world_y(y)
    spawn_effect(world_x, world_y)
}
```

### world_to_screen_x(x)
Converts a world X coordinate to screen X coordinate.

### world_to_screen_y(y)
Converts a world Y coordinate to screen Y coordinate.

```pixel
// Check if a world position is visible on screen
screen_x = world_to_screen_x(enemy.x)
screen_y = world_to_screen_y(enemy.y)

if screen_x >= 0 and screen_x <= window_width() {
    // Enemy is horizontally visible
}
```

## Complete Camera Example

```pixel
player = none
level_width = 2000
level_height = 600

function on_start() {
    create_window(800, 600, "Camera Demo")

    player = create_sprite(load_image("player.png"))
    player.x = 100
    player.y = 400

    // Set up smooth camera follow
    camera_follow(player, 0.1)
}

function on_update(dt) {
    // Player movement
    if key_down(KEY_LEFT) {
        player.x -= 200 * dt
    }
    if key_down(KEY_RIGHT) {
        player.x += 200 * dt
    }

    // Clamp player to level bounds
    player.x = clamp(player.x, 0, level_width)

    // Shake on jump
    if key_pressed(KEY_SPACE) {
        camera_shake(3, 0.1)
    }

    // Zoom with scroll wheel or keys
    if key_pressed(KEY_EQUALS) {
        camera_set_zoom(camera_zoom() + 0.1)
    }
    if key_pressed(KEY_MINUS) {
        camera_set_zoom(camera_zoom() - 0.1)
    }
}

function on_draw() {
    clear(rgb(135, 206, 235))

    // Draw background (scrolls with camera)
    draw_rect(0, 500, level_width, 100, rgb(34, 139, 34))

    // Draw player
    draw_sprite(player)

    // Draw UI (needs to ignore camera - draw at screen coordinates)
    // Note: UI elements should be drawn relative to camera position
    draw_text("Score: 0", camera_x() + 20, camera_y() + 20, default_font(24), WHITE)
}
```

## Camera Tips

1. **Call camera_follow in on_start()** - Set up camera following once, it updates automatically

2. **Use smooth follow for better feel** - A smoothing value of 0.1 works well for most games

3. **Combine with level bounds** - Don't let the camera show outside your level:
   ```pixel
   cam_x = clamp(player.x - 400, 0, level_width - 800)
   cam_y = clamp(player.y - 300, 0, level_height - 600)
   camera_set_position(cam_x, cam_y)
   ```

4. **UI needs offset** - If you have camera movement, add camera position to UI coordinates so they stay fixed on screen

5. **Use screen_to_world for mouse** - Always convert mouse coordinates to world space for proper click detection

## See Also

- [Engine API](/pixel/docs/api/engine) - Window, drawing, sprites
- [Physics API](/pixel/docs/api/physics) - Movement and collision
