# Contributing to Pixel

Thank you for your interest in contributing to Pixel! This document provides guidelines for contributing to the project.

## Getting Started

1. Fork the repository
2. Clone your fork:
   ```bash
   git clone https://github.com/YOUR_USERNAME/pixel.git
   cd pixel
   ```
3. Create a feature branch:
   ```bash
   git checkout -b feature/your-feature
   ```

## Development Setup

### Prerequisites

- C compiler (GCC or Clang)
- CMake 3.16+
- SDL2, SDL2_image, SDL2_mixer, SDL2_ttf (for graphics/audio support)

### Installing Dependencies

**macOS:**
```bash
brew install sdl2 sdl2_image sdl2_ttf sdl2_mixer
```

**Ubuntu/Debian:**
```bash
sudo apt-get install libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev libsdl2-mixer-dev
```

**Windows (vcpkg):**
```bash
vcpkg install sdl2 sdl2-image sdl2-ttf sdl2-mixer
```

### Building

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make
```

### Running Tests

```bash
cd build && ctest --output-on-failure
```

### Running Local CI

Before submitting a pull request, run the local CI script to catch issues early:

```bash
./scripts/ci-local.sh
```

## Code Style

- C11 standard
- 4-space indentation
- Run `.clang-format` for consistent formatting
- Never use emojis in code, comments, or documentation

## Pull Request Guidelines

1. Keep changes focused and atomic
2. Include tests for new functionality
3. Update documentation as needed
4. Ensure all CI checks pass (run `./scripts/ci-local.sh` locally)
5. Write clear commit messages

## Reporting Issues

- Use GitHub Issues for bug reports and feature requests
- Include steps to reproduce for bugs
- Include your OS and build configuration
- For crashes, include any error messages or stack traces

## Project Structure

```
src/
  core/       # Arena allocator, hash table, error handling
  compiler/   # Lexer, parser, AST, semantic analyzer, codegen
  vm/         # Value system, objects, bytecode, VM, GC
  runtime/    # Standard library functions
  engine/     # Game loop, drawing, input, audio
  pal/        # Platform abstraction (SDL2, Emscripten, mock)

tests/unit/   # Unit tests
examples/     # Example Pixel programs
docs/         # Documentation
```

## License

By contributing to Pixel, you agree that your contributions will be licensed under the MIT License.
