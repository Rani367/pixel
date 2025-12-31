---
title: "Getting Started with Pixel"
description: "Create your first 2D game in minutes with Pixel. Learn the basics of game windows, sprites, movement, collision detection, and the game loop."
keywords: ["Pixel tutorial", "first game programming", "beginner game tutorial", "game development quickstart", "learn 2D game development"]
---

# Getting Started with Pixel

Pixel is a beginner-friendly programming language for making 2D games. This guide will help you get up and running in minutes.

## Installation

### Download Binary (Recommended)

Download the latest release for your platform:
- **macOS (Intel)**: `pixel-macos-x64`
- **macOS (Apple Silicon)**: `pixel-macos-arm64`
- **Linux**: `pixel-linux-x64`
- **Windows**: `pixel-windows-x64.exe`

Make the binary executable (macOS/Linux):
```bash
chmod +x pixel
./pixel --version
```

### Build from Source

Requirements: C compiler (gcc or clang), CMake 3.16+, SDL2

```bash
git clone https://github.com/Rani367/pixel.git
cd pixel
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make
```

## Your First Program

Create a file called `hello.pixel`:

```pixel
function on_start() {
    print("Hello, Pixel!")
}
```

Run it:
```bash
./pixel run hello.pixel
```

## Creating a Game Window

Let's create an actual game window:

```pixel
function on_start() {
    create_window(800, 600, "My First Game")
}

function on_draw() {
    clear(rgb(20, 20, 40))
    draw_text("Hello, Pixel!", 300, 280, default_font(32), WHITE)
}
```

This creates an 800x600 window with a dark background and centered text.

## Adding Movement

Let's make something move with keyboard input:

```pixel
x = 400
y = 300
speed = 200

function on_start() {
    create_window(800, 600, "Moving Square")
}

function on_update(dt) {
    if key_down(KEY_LEFT) {
        x = x - speed * dt
    }
    if key_down(KEY_RIGHT) {
        x = x + speed * dt
    }
    if key_down(KEY_UP) {
        y = y - speed * dt
    }
    if key_down(KEY_DOWN) {
        y = y + speed * dt
    }
}

function on_draw() {
    clear(rgb(20, 20, 40))
    draw_rect(x - 25, y - 25, 50, 50, CYAN)
}
```

Key concepts:
- `on_update(dt)` is called every frame with `dt` (delta time) in seconds
- `key_down(KEY)` returns true while a key is held
- Multiply by `dt` for smooth, framerate-independent movement

## Working with Sprites

For more complex graphics, use sprites:

```pixel
player = none

function on_start() {
    create_window(800, 600, "Sprite Demo")
    player = create_sprite(load_image("player.png"))
    player.x = 400
    player.y = 300
}

function on_update(dt) {
    if key_down(KEY_RIGHT) {
        player.x = player.x + 200 * dt
    }
    if key_down(KEY_LEFT) {
        player.x = player.x - 200 * dt
    }
}

function on_draw() {
    clear(rgb(100, 150, 200))
    draw_sprite(player)
}
```

Sprites have properties you can modify:
- `x`, `y` - position
- `rotation` - angle in degrees
- `scale` - size multiplier
- `flip_x`, `flip_y` - mirror the sprite

## Adding Collision

Pixel provides simple collision detection using sprites:

```pixel
player = none
coin = none
score = 0

function on_start() {
    create_window(800, 600, "Collision Demo")

    // Create sprites (can be used without images for collision)
    player = create_sprite(none)
    player.x = 100
    player.y = 300
    player.width = 40
    player.height = 40

    coin = create_sprite(none)
    coin.x = 600
    coin.y = 300
    coin.width = 30
    coin.height = 30
}

function on_update(dt) {
    // Move player
    if key_down(KEY_RIGHT) {
        player.x = player.x + 200 * dt
    }

    // Check collision between sprites
    if collides(player, coin) {
        score = score + 1
        coin.x = random_range(50, 750)
        coin.y = random_range(50, 550)
    }
}

function on_draw() {
    clear(rgb(20, 20, 40))
    draw_rect(player.x, player.y, player.width, player.height, BLUE)
    draw_circle(coin.x + 15, coin.y + 15, 15, YELLOW)
    draw_text("Score: " + to_string(score), 20, 20, default_font(24), WHITE)
}
```

## Core Callbacks

Pixel games use three main callback functions:

| Callback | Purpose | Called |
|----------|---------|--------|
| `on_start()` | Initialize game state | Once at startup |
| `on_update(dt)` | Update game logic | Every frame |
| `on_draw()` | Render graphics | Every frame, after update |

## Next Steps

- Check out the [example games](https://github.com/Rani367/pixel/tree/main/examples/games) for complete working games
- Read the [API Reference](/pixel/docs/api/core) for all available functions
- Read the [Language Guide](/pixel/docs/language/basics) to learn more about Pixel syntax

## Quick Reference

### Input
```pixel
key_down(KEY_SPACE)      // Key is held
key_pressed(KEY_SPACE)   // Key just pressed this frame
mouse_x(), mouse_y()     // Mouse position
mouse_down(MOUSE_LEFT)   // Mouse button held
```

### Drawing
```pixel
clear(color)
draw_rect(x, y, w, h, color)
draw_circle(x, y, radius, color)
draw_line(x1, y1, x2, y2, color)
draw_text(text, x, y, font, color)
```

### Colors
```pixel
rgb(255, 0, 0)           // Red
rgba(0, 255, 0, 128)     // Semi-transparent green
WHITE, BLACK, RED, GREEN, BLUE, YELLOW, CYAN, MAGENTA, ORANGE, GRAY
```

### Math
```pixel
random()                 // 0.0 to 1.0
random_range(10, 20)     // 10.0 to 20.0
sin(angle), cos(angle)   // Angle in radians
sqrt(n), pow(n, exp)
min(a, b), max(a, b), clamp(val, lo, hi)
```
