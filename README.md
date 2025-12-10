# Pixel

A beginner-friendly programming language for making 2D games.

```pixel
function on_start() {
    create_window(800, 600, "My Game")
}

function on_update(dt) {
    if key_down(KEY_RIGHT) {
        player_x = player_x + 200 * dt
    }
}

function on_draw() {
    clear(rgb(20, 20, 40))
    draw_rect(player_x, 300, 40, 40, CYAN)
}
```

## Installation

Download the latest binary for your platform:

- **Windows:** `pixel-windows-x64.exe`
- **macOS (Intel):** `pixel-macos-x64`
- **macOS (Apple Silicon):** `pixel-macos-arm64`
- **Linux:** `pixel-linux-x64`

Make it executable (macOS/Linux):
```bash
chmod +x pixel
```

See [Installation Guide](docs/installation.md) for detailed instructions.

## Quick Start

Create a file called `game.pixel`:

```pixel
player_x = 400
speed = 200

function on_start() {
    create_window(800, 600, "My First Game")
}

function on_update(dt) {
    if key_down(KEY_LEFT) {
        player_x = player_x - speed * dt
    }
    if key_down(KEY_RIGHT) {
        player_x = player_x + speed * dt
    }
}

function on_draw() {
    clear(rgb(20, 20, 40))
    draw_rect(player_x - 20, 280, 40, 40, CYAN)
}
```

Run it:
```bash
pixel run game.pixel
```

## Features

- **Simple syntax** - No semicolons, no type declarations
- **Built-in game loop** - Just define `on_start`, `on_update`, `on_draw`
- **Graphics** - Shapes, images, sprites, text, animations
- **Input** - Keyboard, mouse with event callbacks
- **Audio** - Sound effects and background music
- **Physics** - Collision detection, movement helpers
- **Cross-platform** - Windows, macOS, Linux, and web browsers

## Documentation

- [Getting Started](docs/getting-started.md) - Your first Pixel game
- [Installation](docs/installation.md) - Download and setup
- [Language Guide](docs/language/) - Learn Pixel syntax
  - [Basics](docs/language/basics.md) - Variables and operators
  - [Control Flow](docs/language/control-flow.md) - If statements and loops
  - [Functions](docs/language/functions.md) - Defining functions
  - [Data Structures](docs/language/data-structures.md) - Lists and structs
- [API Reference](docs/api/) - All built-in functions
  - [Core](docs/api/core.md) - Types, I/O, utilities
  - [Math](docs/api/math.md) - Mathematical functions
  - [Engine](docs/api/engine.md) - Window, drawing, sprites
  - [Input](docs/api/input.md) - Keyboard and mouse
  - [Audio](docs/api/audio.md) - Sound and music
  - [Physics](docs/api/physics.md) - Collision and movement

## Examples

Complete games included in `examples/games/`:

| Game | Description |
|------|-------------|
| `pong.pixel` | Two-player Pong with scoring |
| `breakout.pixel` | Classic brick-breaking game |
| `platformer.pixel` | Side-scrolling with coins and enemies |
| `shooter.pixel` | Top-down wave shooter |

Run an example:
```bash
pixel run examples/games/pong.pixel
```

## Building from Source

```bash
git clone https://github.com/user/pixel.git
cd pixel
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

Requirements: C compiler, CMake 3.16+, SDL2

## License

MIT
