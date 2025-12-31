---
title: "Pixel Documentation"
description: "Official documentation for Pixel, a beginner-friendly programming language for making 2D games. Learn to create games with simple syntax and built-in game engine features."
keywords: ["Pixel documentation", "game programming tutorial", "beginner game development", "2D game engine docs", "learn game programming"]
---

# Pixel Documentation

Pixel is a beginner-friendly programming language designed for making 2D games. Write simple code, see immediate results.

## Quick Links

- [Installation](/pixel/docs/installation) - Download and install Pixel
- [Getting Started](/pixel/docs/getting-started) - Create your first game in minutes
- [Language Guide](/pixel/docs/language/basics) - Learn the Pixel language
- [API Reference](/pixel/docs/api/core) - Complete function reference
- [Examples](https://github.com/Rani367/pixel/tree/main/examples) - Sample games and code

## What is Pixel?

Pixel is a scripting language with a built-in game engine. You write code in `.pixel` files and run them with the `pixel` command. The language handles window creation, graphics, input, and audio so you can focus on making games.

```pixel
function on_start() {
    create_window(800, 600, "Hello Pixel")
}

function on_draw() {
    clear(rgb(20, 20, 40))
    draw_text("Hello, World!", 300, 280, default_font(32), WHITE)
}
```

## Features

**Simple Syntax** - No semicolons, no type declarations, minimal boilerplate

**Game Loop Built-in** - Define `on_start`, `on_update`, `on_draw` and Pixel handles the rest

**Batteries Included** - Drawing, sprites, audio, input, physics, and collision detection

**Cross-Platform** - Runs on Windows, macOS, Linux, and web browsers

## Documentation Structure

### Language Guide
Learn Pixel's syntax and features:
- [Basics](/pixel/docs/language/basics) - Variables, types, and operators
- [Control Flow](/pixel/docs/language/control-flow) - Conditionals and loops
- [Functions](/pixel/docs/language/functions) - Defining and calling functions
- [Data Structures](/pixel/docs/language/data-structures) - Lists and structs

### API Reference
Complete reference for all built-in functions:
- [Core](/pixel/docs/api/core) - Type conversion, I/O, utilities
- [Math](/pixel/docs/api/math) - Mathematical functions
- [Engine](/pixel/docs/api/engine) - Window, drawing, sprites, camera
- [Input](/pixel/docs/api/input) - Keyboard and mouse
- [Audio](/pixel/docs/api/audio) - Sound effects and music
- [Physics](/pixel/docs/api/physics) - Collision detection and movement

### Examples
Complete games to learn from:
- `examples/games/pong.pixel` - Classic two-player Pong
- `examples/games/breakout.pixel` - Brick-breaking game
- `examples/games/platformer.pixel` - Side-scrolling platformer
- `examples/games/shooter.pixel` - Top-down wave shooter
