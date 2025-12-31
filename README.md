# Pixel

[![Coverage](https://img.shields.io/endpoint?url=https://gist.githubusercontent.com/Rani367/45a4bd6cbc1add6850e45c3206976183/raw/coverage.json)](https://github.com/Rani367/pixel/actions)

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

**One command install (macOS/Linux):**
```bash
curl -fsSL https://raw.githubusercontent.com/Rani367/pixel/main/scripts/install.sh | bash
```

This automatically installs all dependencies and adds `pixel` to your PATH.

See [Installation Guide](docs/installation.md) for other options.

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
pixel game.pixel
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

### Demo Programs

Feature demonstrations in `examples/`:

| Demo | Description |
|------|-------------|
| `audio_demo.pixel` | Sound effects and music playback |
| `input_demo.pixel` | Keyboard and mouse input handling |
| `physics_demo.pixel` | Gravity, collision, and movement |
| `advanced_features.pixel` | Closures, structs, and more |
| `web_demo.pixel` | Web-specific features |

### Complete Games

Full games in `examples/games/`:

| Game | Description |
|------|-------------|
| `pong.pixel` | Two-player Pong with scoring |
| `breakout.pixel` | Classic brick-breaking game |
| `platformer.pixel` | Side-scrolling with coins and enemies |
| `shooter.pixel` | Top-down wave shooter |

Run an example:
```bash
pixel examples/games/pong.pixel
```

## Building from Source

```bash
git clone https://github.com/Rani367/pixel.git
cd pixel
./scripts/install.sh
```

The install script handles all dependencies automatically.

## License

MIT
