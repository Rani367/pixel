# Changelog

All notable changes to Pixel will be documented in this file.

## [1.0.0] - 2025-01-01

### Language Features
- Dynamic typing with numbers, strings, booleans, lists, structs, and vec2
- First-class functions with closures
- Control flow: if/else, while loops, break/continue
- Struct literals with property access
- String interpolation and operations (substring, split, join, upper, lower)
- List operations (push, pop, insert, remove, contains, index_of)

### Standard Library
- Math: abs, floor, ceil, round, min, max, clamp, sqrt, pow
- Trigonometry: sin, cos, tan, atan2
- Random: random, random_range, random_int
- Type conversion: to_string, to_number, type
- Utility: print, println, range, time, clock

### Game Engine
- Window creation and management
- Drawing primitives: rectangles, circles, lines
- Image loading and sprite system
- Sprite properties: position, rotation, scale, flip, origin
- Sprite sheets and animation support
- Font loading and text rendering
- Camera system with follow, shake, and zoom
- Particle system with emitters
- Scene management with per-scene callbacks

### Input System
- Keyboard: key_down, key_pressed, key_released
- Mouse: position, button states
- Full key constant support (letters, numbers, arrows, function keys, modifiers)
- Input callbacks: on_key_down, on_key_up, on_mouse_click, on_mouse_move

### Audio System
- Sound effect loading and playback
- Music playback with loop support
- Volume control (master, music, per-sound)
- Pause/resume/stop controls

### Physics & Collision
- Sprite physics properties (velocity, acceleration, friction, gravity)
- Collision detection: AABB, circle, point-in-rect
- Physics helpers: apply_force, move_toward, look_at
- Math utilities: lerp, lerp_angle, distance

### Developer Experience
- Pretty error messages with source code context and underlines
- "Did you mean?" suggestions for undefined variables
- Bytecode disassembler for debugging
- Bytecode serialization (.plbc format)

### Platforms
- macOS (Intel x64 and Apple Silicon)
- Linux (x64)
- Windows (x64)
- Web (Emscripten/WebAssembly)

### Example Games
- Pong: Two-player classic with paddle physics
- Breakout: Brick-breaking with lives and scoring
- Platformer: Side-scrolling with coins and enemies
- Top-Down Shooter: Wave-based survival with multiple enemy types
