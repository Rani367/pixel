---
title: "API Reference"
description: "Complete API reference for Pixel programming language. Documentation for all built-in functions including graphics, input, audio, physics, and more."
keywords: ["Pixel API", "function reference", "game development API", "Pixel documentation"]
---

# API Reference

Complete reference for all built-in functions in Pixel.

## Core Language

| Reference | Description |
|-----------|-------------|
| [Core](/pixel/docs/api/core) | Type conversion, I/O, strings, lists, utilities |
| [Math](/pixel/docs/api/math) | Mathematical functions, trigonometry, random numbers |

## Game Engine

| Reference | Description |
|-----------|-------------|
| [Engine](/pixel/docs/api/engine) | Window, drawing, sprites, images, fonts |
| [Input](/pixel/docs/api/input) | Keyboard and mouse input |
| [Audio](/pixel/docs/api/audio) | Sound effects and music |
| [Physics](/pixel/docs/api/physics) | Collision detection, physics properties |
| [Animation](/pixel/docs/api/animation) | Sprite sheet animations |
| [Camera](/pixel/docs/api/camera) | Camera position, zoom, shake, follow |
| [Particles](/pixel/docs/api/particles) | Particle effects and emitters |
| [Scenes](/pixel/docs/api/scenes) | Scene management and transitions |

## Quick Function Index

### Core Functions
- `print()`, `println()` - Output
- `type()`, `to_string()`, `to_number()` - Type handling
- `len()`, `push()`, `pop()`, `insert()`, `remove()` - List operations
- `substring()`, `split()`, `join()`, `upper()`, `lower()` - String operations
- `range()`, `time()`, `clock()` - Utilities

### Math Functions
- `abs()`, `floor()`, `ceil()`, `round()` - Rounding
- `min()`, `max()`, `clamp()` - Clamping
- `sqrt()`, `pow()` - Powers and roots
- `sin()`, `cos()`, `tan()`, `atan2()` - Trigonometry
- `random()`, `random_range()`, `random_int()` - Random numbers
- `lerp()`, `lerp_angle()` - Interpolation

### Engine Functions
- `create_window()`, `window_width()`, `window_height()` - Window
- `clear()`, `draw_rect()`, `draw_circle()`, `draw_line()` - Drawing
- `load_image()`, `draw_image()` - Images
- `create_sprite()`, `draw_sprite()` - Sprites
- `default_font()`, `load_font()`, `draw_text()` - Text

### Input Functions
- `key_down()`, `key_pressed()`, `key_released()` - Keyboard
- `mouse_x()`, `mouse_y()` - Mouse position
- `mouse_down()`, `mouse_pressed()`, `mouse_released()` - Mouse buttons

### Audio Functions
- `load_sound()`, `play_sound()` - Sound effects
- `load_music()`, `play_music()`, `play_music_loop()` - Music
- `pause_music()`, `resume_music()`, `stop_music()` - Music control
- `set_music_volume()`, `set_master_volume()` - Volume

### Physics Functions
- `collides()`, `collides_rect()`, `collides_point()`, `collides_circle()` - Collision
- `distance()` - Distance between sprites
- `apply_force()`, `move_toward()`, `look_at()` - Movement helpers
- `set_gravity()`, `get_gravity()` - Global gravity

### Animation Functions
- `create_animation()` - Create from sprite sheet
- `sprite_set_animation()`, `sprite_play()`, `sprite_stop()` - Playback

### Camera Functions
- `camera_x()`, `camera_y()`, `camera_zoom()` - Get position/zoom
- `camera_set_position()`, `camera_set_zoom()` - Set position/zoom
- `camera_follow()`, `camera_shake()` - Effects
- `screen_to_world_x/y()`, `world_to_screen_x/y()` - Coordinate conversion

### Particle Functions
- `create_emitter()`, `emitter_emit()` - Create and emit
- `emitter_set_color/size/speed/lifetime/angle/gravity/rate/position/active()` - Configure
- `draw_particles()` - Render

### Scene Functions
- `load_scene()`, `get_scene()` - Scene control

## See Also

- [Language Guide](/pixel/docs/language/basics) - Learn Pixel syntax
- [Getting Started](/pixel/docs/getting-started) - Your first game
