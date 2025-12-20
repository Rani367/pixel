it executable:

```bash
chmod +x pixel-linux-x64
./pixel-linux-x64 run game.pixel
```

To install system-wide:
```bash
sudo mv pixel-linux-x64 /usr/local/bin/pixel
pixel run game.pixel
```

## Building from Source

### Requirements

- C compiler (GCC or Clang)
- CMake 3.16 or later
- SDL2, SDL2_image, SDL2_ttf, SDL2_mixer (for graphics/audio)

### Install Dependencies

**macOS:**
```bash
brew install cmake sdl2 sdl2_image sdl2_ttf sdl2_mixer
```

**Ubuntu/Debian:**
```bash
sudo apt install cmake libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev libsdl2-mixer-dev
```

**Windows (vcpkg):**
```cmd
vcpkg install sdl2 sdl2-image sdl2-ttf sdl2-mixer
```

### Build

```bash
git clone https://github.com/Rani367/pixel.git
cd pixel
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

The `pixel` executable will be in the `build` directory.

### Verify Installation

```bash
pixel version
```

Should output: `Pixel 1.0.0`

## Web Build (Optional)

Pixel games can be compiled to run in web browsers using Emscripten.

### Install Emscripten

```bash
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh
```

### Build for Web

```bash
./scripts/build-web.sh
```

### Test in Browser

```bash
cd build-web
python3 -m http.server 8000
# Open http://localhost:8000/pixel.html
```

## Next Steps

- [Getting Started](/pixel/docs/getting-started) - Create your first game
- [Language Guide](/pixel/docs/language/basics) - Learn Pixel syntax
# Installation

## Download

Download the latest release for your platform:

| Platform | Download |
|----------|----------|
| Windows (64-bit) | `pixel-windows-x64.exe` |
| macOS (Intel) | `pixel-macos-x64` |
| macOS (Apple Silicon) | `pixel-macos-arm64` |
| Linux (64-bit) | `pixel-linux-x64` |

## Setup

### Windows

1. Download `pixel-windows-x64.exe`
2. Rename to `pixel.exe` (optional)
3. Add to your PATH or run from the download directory

```cmd
pixel.exe run game.pixel
```

### macOS

1. Download the appropriate binary for your Mac
2. Make it executable and run:

```bash
chmod +x pixel-macos-*
./pixel-macos-arm64 run game.pixel
```

To install system-wide:
```bash
sudo mv pixel-macos-arm64 /usr/local/bin/pixel
pixel run game.pixel
```

### Linux

1. Download `pixel-linux-x64`
2. Make 