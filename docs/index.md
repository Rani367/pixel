---
title: "Pixel Documentation"
description: "Official documentation for Pixel, a beginner-friendly programming language for making 2D games. Learn to create games with simple syntax and built-in game engine features."
keywords: ["Pixel documentation", "game programming tutorial", "beginner game development", "2D game engine docs", "learn game programming"]
---

# Pixel Documentation

Pixel is a beginner-friendly programming language designed for making 2D games. Write simple code, see immediate results.

## Quick Start

1. **[Install Pixel](/pixel/docs/installation)** - One command to get started
2. **[Create Your First Game](/pixel/docs/getting-started)** - Build a game in minutes
3. **[Learn the Language](/pixel/docs/language/)** - Master Pixel syntax

## What is Pixel?

Pixel is a scripting language with a built-in game engine. You write code in `.pixel` files and run them with the `pixel` command. The language handles window creation, graphics, input, and audio so you can focus on making games.

```pixel
player_x = 400

function on_start() {
    create_window(800, 600, "Hello Pixel")
}

function on_update(dt) {
    if key_down(KEY_RIGHT) {
        player_x += 200 * dt
    }
}

function on_draw() {
    clear(rgb(20, 20, 40))
    draw_rect(player_x - 20, 280, 40, 40, CYAN)
}
```

## Features

- **Simple Syntax** - No semicolons, no type declarations, minimal boilerplate
- **Game Loop Built-in** - Define `on_start`, `on_update`, `on_draw` and Pixel handles the rest
- **Batteries Included** - Drawing, sprites, audio, input, physics, and collision detection
- **Cross-Platform** - Runs on Windows, macOS, Linux, and web browsers

## Documentation

### Getting Started
- [Installation](/pixel/docs/installation) - Download and install Pixel
- [Getting Started](/pixel/docs/getting-started) - Create your first game in minutes

### Language Guide
Learn Pixel's syntax and features:
- [Basics](/pixel/docs/language/basics) - Variables, types, and operators
- [Control Flow](/pixel/docs/language/control-flow) - Conditionals, loops, break/continue
- [Functions](/pixel/docs/language/functions) - Defining functions, closures, callbacks
- [Data Structures](/pixel/docs/language/data-structures) - Lists, structs, and vectors

### API Reference
Complete reference for all built-in functions:
- [Core](/pixel/docs/api/core) - Type conversion, I/O, strings, lists
- [Math](/pixel/docs/api/math) - Mathematical functions, trigonometry, random
- [Engine](/pixel/docs/api/engine) - Window, drawing, sprites, fonts
- [Input](/pixel/docs/api/input) - Keyboard and mouse
- [Audio](/pixel/docs/api/audio) - Sound effects and music
- [Physics](/pixel/docs/api/physics) - Collision detection and movement
- [Animation](/pixel/docs/api/animation) - Sprite sheet animations
- [Camera](/pixel/docs/api/camera) - Camera position, zoom, shake
- [Particles](/pixel/docs/api/particles) - Particle effects
- [Scenes](/pixel/docs/api/scenes) - Scene management

### Guides
In-depth guides for common topics:
- [Understanding the Game Loop](/pixel/docs/guides/game-loop) - How on_start, on_update, and on_draw work
- [Debugging](/pixel/docs/guides/debugging) - Find and fix problems
- [Structuring Larger Games](/pixel/docs/guides/structuring-games) - Organize your code

### Examples
Complete games to learn from:
- `examples/games/pong.pixel` - Classic two-player Pong
- `examples/games/breakout.pixel` - Brick-breaking game
- `examples/games/platformer.pixel` - Side-scrolling platformer
- `examples/games/shooter.pixel` - Top-down wave shooter

View all examples on [GitHub](https://github.com/Rani367/pixel/tree/main/examples).
