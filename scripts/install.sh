#!/bin/bash
# Install Pixel programming language
#
# Usage:
#   ./scripts/install.sh           # Install to ~/.local/bin (no sudo)
#   sudo ./scripts/install.sh      # Install to /usr/local/bin
#   ./scripts/install.sh -y        # Non-interactive (skip prompts)
#
# After installation, you can run Pixel scripts with:
#   pixel game.pixel

set -e

# Parse arguments
FORCE=false
for arg in "$@"; do
    case $arg in
        -y|--yes)
            FORCE=true
            ;;
    esac
done

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

print_step() {
    echo -e "${BLUE}==>${NC} $1"
}

print_success() {
    echo -e "${GREEN}==>${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}Warning:${NC} $1"
}

print_error() {
    echo -e "${RED}Error:${NC} $1"
}

# Get the project root directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

# Determine install location based on privileges
if [ "$EUID" -eq 0 ]; then
    INSTALL_DIR="/usr/local/bin"
else
    INSTALL_DIR="$HOME/.local/bin"
fi

echo ""
echo "  ____  _          _ "
echo " |  _ \(_)_  _____| |"
echo " | |_) | \ \/ / _ \ |"
echo " |  __/| |>  <  __/ |"
echo " |_|   |_/_/\_\___|_|"
echo ""
echo " Pixel Programming Language Installer"
echo ""

# Check for required tools
print_step "Checking dependencies..."

# Check for C compiler
if ! command -v cc &> /dev/null && ! command -v gcc &> /dev/null && ! command -v clang &> /dev/null; then
    print_error "No C compiler found. Please install gcc or clang."
    echo ""
    echo "  macOS:   xcode-select --install"
    echo "  Ubuntu:  sudo apt install build-essential"
    echo "  Fedora:  sudo dnf install gcc"
    exit 1
fi

# Check for CMake
if ! command -v cmake &> /dev/null; then
    print_error "CMake not found. Please install CMake 3.16 or later."
    echo ""
    echo "  macOS:   brew install cmake"
    echo "  Ubuntu:  sudo apt install cmake"
    echo "  Fedora:  sudo dnf install cmake"
    exit 1
fi

# Check CMake version
CMAKE_VERSION=$(cmake --version | head -n1 | grep -oE '[0-9]+\.[0-9]+')
CMAKE_MAJOR=$(echo "$CMAKE_VERSION" | cut -d. -f1)
CMAKE_MINOR=$(echo "$CMAKE_VERSION" | cut -d. -f2)
if [ "$CMAKE_MAJOR" -lt 3 ] || ([ "$CMAKE_MAJOR" -eq 3 ] && [ "$CMAKE_MINOR" -lt 16 ]); then
    print_error "CMake version 3.16 or later is required (found $CMAKE_VERSION)"
    exit 1
fi

# Check for SDL2 (optional but recommended)
SDL2_FOUND=true
if ! pkg-config --exists sdl2 2>/dev/null; then
    SDL2_FOUND=false
fi

if [ "$SDL2_FOUND" = false ]; then
    print_warning "SDL2 not found. Graphics will be disabled (mock backend)."
    echo ""
    echo "  To enable graphics, install SDL2 and its extensions:"
    echo ""
    case "$(uname -s)" in
        Darwin)
            echo "    brew install sdl2 sdl2_image sdl2_ttf sdl2_mixer"
            ;;
        Linux)
            echo "    Ubuntu/Debian: sudo apt install libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev libsdl2-mixer-dev"
            echo "    Fedora:        sudo dnf install SDL2-devel SDL2_image-devel SDL2_ttf-devel SDL2_mixer-devel"
            ;;
    esac
    echo ""
    if [ "$FORCE" = false ]; then
        read -p "  Continue without graphics support? [y/N] " -n 1 -r
        echo ""
        if [[ ! $REPLY =~ ^[Yy]$ ]]; then
            exit 1
        fi
    else
        echo "  Continuing without graphics support (-y flag set)"
    fi
fi

print_success "Dependencies OK"

# Build
print_step "Building Pixel..."

BUILD_DIR="$PROJECT_ROOT/build"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

cmake "$PROJECT_ROOT" -DCMAKE_BUILD_TYPE=Release > /dev/null

# Detect number of CPU cores
if command -v nproc &> /dev/null; then
    JOBS=$(nproc)
elif command -v sysctl &> /dev/null; then
    JOBS=$(sysctl -n hw.ncpu)
else
    JOBS=4
fi

cmake --build . --parallel "$JOBS" > /dev/null

if [ ! -f "$BUILD_DIR/pixel" ]; then
    print_error "Build failed - pixel executable not found"
    exit 1
fi

print_success "Build complete"

# Install
print_step "Installing to $INSTALL_DIR..."

mkdir -p "$INSTALL_DIR"
cp "$BUILD_DIR/pixel" "$INSTALL_DIR/pixel"
chmod +x "$INSTALL_DIR/pixel"

print_success "Installed successfully!"

# Check if install dir is in PATH
if [[ ":$PATH:" != *":$INSTALL_DIR:"* ]]; then
    echo ""
    print_warning "$INSTALL_DIR is not in your PATH"
    echo ""
    echo "  Add it to your shell configuration:"
    echo ""

    SHELL_NAME=$(basename "$SHELL")
    case "$SHELL_NAME" in
        zsh)
            echo "    echo 'export PATH=\"$INSTALL_DIR:\$PATH\"' >> ~/.zshrc"
            echo "    source ~/.zshrc"
            ;;
        bash)
            echo "    echo 'export PATH=\"$INSTALL_DIR:\$PATH\"' >> ~/.bashrc"
            echo "    source ~/.bashrc"
            ;;
        fish)
            echo "    fish_add_path $INSTALL_DIR"
            ;;
        *)
            echo "    export PATH=\"$INSTALL_DIR:\$PATH\""
            ;;
    esac
    echo ""
fi

# Verify installation
if command -v pixel &> /dev/null || [ -x "$INSTALL_DIR/pixel" ]; then
    echo ""
    print_success "Installation complete!"
    echo ""
    echo "  Verify with:  pixel version"
    echo "  Run a game:   pixel game.pixel"
    echo ""
else
    echo ""
    print_success "Binary installed to: $INSTALL_DIR/pixel"
    echo ""
    echo "  After updating your PATH, run:"
    echo "    pixel version"
    echo ""
fi
