pixel) → Lexer → Parser → AST → Analyzer → Bytecode → VM
                                                                 ↓
                                                     Platform Abstraction Layer
                                                     (SDL2 / Web/WASM / Mock)
```

### Directory Structure

```
src/
├── core/       # Arena allocator, hash table, error handling, strings
├── compiler/   # Lexer, parser, AST, semantic analyzer, bytecode generator
├── vm/         # Value system, objects, bytecode chunks, VM, garbage collector
├── runtime/    # Standard library (math, string, list functions)
├── engine/     # Game loop, callbacks, drawing, input, audio natives
└── pal/        # Platform abstraction (SDL2, Emscripten, mock backends)

docs/
├── index.md           # Documentation home
├── installation.md    # Install instructions
├── getting-started.md # Tutorial
├── language/          # Language reference
└── api/               # API reference

examples/
└── games/      # Complete example games (pong, breakout, platformer, shooter)

tests/
└── unit/       # Unit tests for all components

benchmarks/     # Performance benchmarks
```

### Key Components

| Directory | Purpose |
|-----------|---------|
| `src/core/` | Foundational utilities (arena allocator, dynamic array, hash table, error handling) |
| `src/compiler/` | Front-end pipeline (lexer, parser, AST, analyzer, codegen) |
| `src/vm/` | Execution engine (values, objects, chunks, VM, GC, natives) |
| `src/runtime/` | Standard library functions |
| `src/engine/` | Game engine (loop, callbacks, drawing, input, audio) |
| `src/pal/` | Platform abstraction layer (SDL2, Emscripten, mock) |

### Memory Management

- **Arena allocator**: Used for AST nodes during compilation (batch allocation/deallocation)
- **Garbage collector**: Tri-color mark-and-sweep for runtime objects

### Value System

Values use a tagged union representation. All heap objects share a common header with type, GC mark bit, and intrusive linked list pointer. Strings are interned for O(1) equality comparison.

## Language Syntax

```pixel
function on_start() {
    create_window(800, 600, "My Game")
    player = create_sprite(load_image("hero.png"))
    player.x = 400
}

function on_update(dt) {
    if key_down(KEY_RIGHT) {
        player.x = player.x + 200 * dt
    }
}

function on_draw() {
    clear(rgb(20, 20, 40))
    draw_sprite(player)
}
```

## File Extensions

- `.pixel` - Source files
- `.plbc` - Compiled bytecode

## Testing

```bash
# Run all tests
cd build && ctest --output-on-failure

# Run specific test
./build/tests/test_vm
```

Test files are in `tests/unit/`. Each component has its own test file (test_lexer.c, test_parser.c, test_vm.c, etc.).

## Code Style

- C11 standard
- 4-space indentation
- Never use emojis in code, comments, or documentation
- Run `.clang-format` for consistent formatting
ilure

# Run CI checks locally (ALWAYS run before pushing)
./scripts/ci-local.sh
```

> **Note to Claude:** Always run `./scripts/ci-local.sh` before pushing to GitHub to catch build/test failures early.

## GitHub CI Verification

After pushing changes to GitHub, always verify that CI passes:

```bash
# Push changes
git push origin main

# Watch the CI run and wait for completion
gh run watch --exit-status

# If CI fails, check the logs
gh run view --log-failed
```

> **Note to Claude:** After every push, you MUST watch the GitHub CI run using `gh run watch --exit-status`. If it fails, examine the logs with `gh run view --log-failed`, fix the issues, and repeat until all CI checks pass. Do not consider the task complete until GitHub CI is green. Common CI-only failures include:
> - GCC-specific warnings (Ubuntu uses GCC, not Clang)
> - Memory leaks detected by LeakSanitizer (only enabled in Debug builds on CI)
> - Platform-specific issues between macOS and Linux

## Architecture

The system follows a traditional compiler pipeline with a game engine layer:

```
Source (.# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Pixel is a beginner-friendly programming language for making 2D games. It compiles to bytecode and runs on a custom virtual machine with an integrated game engine.

**Version:** 1.0.0

## Build Commands

```bash
# Build (from project root)
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug  # or Release
make

# Run the interpreter
./build/pixel run <file.pixel>

# Run tests
cd build && ctest --output-on-fa