---
title: "Animation API Reference"
description: "Create sprite animations in Pixel using sprite sheets. Learn to define animations, control playback, and animate game characters."
keywords: ["Pixel animation", "sprite animation", "sprite sheets", "game animation", "frame animation"]
---

# Animation API Reference

Pixel supports sprite sheet animations for creating animated game characters, effects, and UI elements.

## Creating Animations

### create_animation(image, frame_width, frame_height, frames, frame_time)
Creates an animation from a sprite sheet image.

| Parameter | Type | Description |
|-----------|------|-------------|
| `image` | Image | The sprite sheet image |
| `frame_width` | Number | Width of each frame in pixels |
| `frame_height` | Number | Height of each frame in pixels |
| `frames` | List | List of frame indices to play |
| `frame_time` | Number | Duration of each frame in seconds |

Frames are numbered left-to-right, top-to-bottom starting at 0.

```pixel
// Load a sprite sheet with 4 frames in a row
sheet = load_image("player_walk.png")

// Create animation using frames 0, 1, 2, 3 at 0.15 seconds each
walk_anim = create_animation(sheet, 32, 32, [0, 1, 2, 3], 0.15)
```

### Sprite Sheet Layout

For a 128x32 sprite sheet with 32x32 frames:
```
+---+---+---+---+
| 0 | 1 | 2 | 3 |   <- Frame indices
+---+---+---+---+
```

For a 64x64 sprite sheet with 32x32 frames:
```
+---+---+
| 0 | 1 |
+---+---+
| 2 | 3 |
+---+---+
```

## Assigning Animations to Sprites

### sprite_set_animation(sprite, animation)
Assigns an animation to a sprite. The sprite will display the animation frames.

```pixel
player = create_sprite(none)
player.x = 400
player.y = 300

walk_anim = create_animation(sheet, 32, 32, [0, 1, 2, 3], 0.15)
sprite_set_animation(player, walk_anim)
```

## Animation Playback Control

### sprite_play(sprite)
Starts or resumes playing the sprite's animation.

```pixel
sprite_play(player)
```

### sprite_stop(sprite)
Stops the sprite's animation at the current frame.

```pixel
sprite_stop(player)
```

## Manual Frame Control

### set_sprite_frame(sprite, frame_index)
Manually sets the sprite's current frame index.

```pixel
// Show the first frame
set_sprite_frame(player, 0)

// Show the third frame
set_sprite_frame(player, 2)
```

This is useful for:
- Static sprite sheets without animation
- Manual animation control
- Preview specific frames

## Sprite Sheet Properties

When using sprite sheets, you can also set these sprite properties directly:

| Property | Type | Description |
|----------|------|-------------|
| `frame_x` | Number | X offset in the sheet (pixels) |
| `frame_y` | Number | Y offset in the sheet (pixels) |
| `frame_width` | Number | Width of the frame (pixels) |
| `frame_height` | Number | Height of the frame (pixels) |

```pixel
// Manual sprite sheet setup
player = create_sprite(load_image("characters.png"))
player.frame_x = 64      // Start at x=64
player.frame_y = 0       // First row
player.frame_width = 32  // 32 pixels wide
player.frame_height = 32 // 32 pixels tall
```

## Complete Animation Example

```pixel
player = none
walk_anim = none
idle_anim = none
is_walking = false

function on_start() {
    create_window(800, 600, "Animation Demo")

    // Load sprite sheet
    sheet = load_image("player.png")

    // Create animations
    idle_anim = create_animation(sheet, 32, 32, [0], 1.0)
    walk_anim = create_animation(sheet, 32, 32, [1, 2, 3, 4], 0.1)

    // Create player sprite
    player = create_sprite(none)
    player.x = 400
    player.y = 300
    sprite_set_animation(player, idle_anim)
    sprite_play(player)
}

function on_update(dt) {
    // Check for movement input
    moving = key_down(KEY_LEFT) or key_down(KEY_RIGHT)

    // Switch animation based on state
    if moving and not is_walking {
        sprite_set_animation(player, walk_anim)
        sprite_play(player)
        is_walking = true
    } else if not moving and is_walking {
        sprite_set_animation(player, idle_anim)
        sprite_play(player)
        is_walking = false
    }

    // Move player
    if key_down(KEY_LEFT) {
        player.x -= 200 * dt
        player.flip_x = true
    }
    if key_down(KEY_RIGHT) {
        player.x += 200 * dt
        player.flip_x = false
    }
}

function on_draw() {
    clear(rgb(20, 20, 40))
    draw_sprite(player)
}
```

## Animation Tips

1. **Keep frame times consistent** - Use the same frame_time for smooth animations (0.1 to 0.2 seconds is common)

2. **Use `flip_x` for directions** - Instead of creating separate left/right animations, flip the sprite:
   ```pixel
   player.flip_x = true   // Face left
   player.flip_x = false  // Face right
   ```

3. **Organize your sprite sheets** - Group related animations on the same sheet

4. **Loop vs. One-shot** - Walk cycles should loop, attack animations might not

## See Also

- [Engine API](/pixel/docs/api/engine) - Sprites, images, and drawing
- [Input API](/pixel/docs/api/input) - Keyboard and mouse input
