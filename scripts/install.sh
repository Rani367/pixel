#!/bin/bash
# Install Pixel programming language
#
# This script automatically installs all dependencies and Pixel itself.
#
# Usage:
#   curl -fsSL https://raw.githubusercontent.com/Rani367/pixel/main/scripts/install.sh | bash
#
# Or clone and run locally:
#   git clone https://github.com/Rani367/pixel.git
#   cd pixel
#   ./scripts/install.sh
#
# After installation, you can run Pixel scripts with:
#   pixel game.pixel

set -e

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

# Detect OS
detect_os() {
    case "$(uname -s)" in
        Darwin)
            OS="macos"
            ;;
        Linux)
            if [ -f /etc/debian_version ]; then
                OS="debian"
            elif [ -f /etc/fedora-release ]; then
                OS="fedora"
            elif [ -f /etc/arch-release ]; then
                OS="arch"
            else
                OS="linux"
            fi
            ;;
        *)
            OS="unknown"
            ;;
    esac
}

# Install Homebrew on macOS if needed
install_homebrew() {
    if ! command -v brew &> /dev/null; then
        print_step "Installing Homebrew..."
        /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

        # Add brew to PATH for this session
        if [ -f /opt/homebrew/bin/brew ]; then
            eval "$(/opt/homebrew/bin/brew shellenv)"
        elif [ -f /usr/local/bin/brew ]; then
            eval "$(/usr/local/bin/brew shellenv)"
        fi
    fi
}

# Install dependencies based on OS
install_dependencies() {
    print_step "Installing dependencies..."

    case "$OS" in
        macos)
            install_homebrew

            # Install all dependencies via brew
            PACKAGES=""
            if ! command -v cmake &> /dev/null; then
                PACKAGES="$PACKAGES cmake"
            fi
            if ! brew list sdl2 &> /dev/null; then
                PACKAGES="$PACKAGES sdl2 sdl2_image sdl2_ttf sdl2_mixer"
            fi

            if [ -n "$PACKAGES" ]; then
                print_step "Installing:$PACKAGES"
                brew install $PACKAGES
            fi

            # Ensure Xcode command line tools are installed
            if ! xcode-select -p &> /dev/null; then
                print_step "Installing Xcode Command Line Tools..."
                xcode-select --install 2>/dev/null || true
                echo "Please complete the Xcode Command Line Tools installation and re-run this script."
                exit 1
            fi
            ;;

        debian)
            print_step "Updating package lists..."
            sudo apt-get update -qq

            print_step "Installing build tools and SDL2..."
            sudo apt-get install -y -qq \
                build-essential \
                cmake \
                libsdl2-dev \
                libsdl2-image-dev \
                libsdl2-ttf-dev \
                libsdl2-mixer-dev
            ;;

        fedora)
            print_step "Installing build tools and SDL2..."
            sudo dnf install -y \
                gcc \
                cmake \
                SDL2-devel \
                SDL2_image-devel \
                SDL2_ttf-devel \
                SDL2_mixer-devel
            ;;

        arch)
            print_step "Installing build tools and SDL2..."
            sudo pacman -S --needed --noconfirm \
                base-devel \
                cmake \
                sdl2 \
                sdl2_image \
                sdl2_ttf \
                sdl2_mixer
            ;;

        *)
            print_error "Unsupported operating system."
            echo ""
            echo "Please install manually:"
            echo "  - C compiler (gcc or clang)"
            echo "  - CMake 3.16+"
            echo "  - SDL2, SDL2_image, SDL2_ttf, SDL2_mixer"
            exit 1
            ;;
    esac

    print_success "Dependencies installed"
}

# Clone or update repository
setup_repository() {
    # Check if we're already in the pixel directory
    if [ -f "CMakeLists.txt" ] && grep -q "project(pixel" CMakeLists.txt 2>/dev/null; then
        PROJECT_ROOT="$(pwd)"
        print_step "Using existing Pixel repository"
    elif [ -f "../CMakeLists.txt" ] && grep -q "project(pixel" ../CMakeLists.txt 2>/dev/null; then
        PROJECT_ROOT="$(cd .. && pwd)"
        print_step "Using existing Pixel repository"
    else
        # Clone the repository
        CLONE_DIR="$HOME/.pixel-src"
        if [ -d "$CLONE_DIR/.git" ]; then
            print_step "Updating Pixel repository..."
            cd "$CLONE_DIR"
            git pull --quiet
        else
            print_step "Cloning Pixel repository..."
            rm -rf "$CLONE_DIR"
            git clone --quiet https://github.com/Rani367/pixel.git "$CLONE_DIR"
            cd "$CLONE_DIR"
        fi
        PROJECT_ROOT="$CLONE_DIR"
    fi
}

# Build Pixel
build_pixel() {
    print_step "Building Pixel..."

    BUILD_DIR="$PROJECT_ROOT/build"
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"

    cmake "$PROJECT_ROOT" -DCMAKE_BUILD_TYPE=Release > /dev/null 2>&1

    # Detect number of CPU cores
    if command -v nproc &> /dev/null; then
        JOBS=$(nproc)
    elif command -v sysctl &> /dev/null; then
        JOBS=$(sysctl -n hw.ncpu)
    else
        JOBS=4
    fi

    cmake --build . --parallel "$JOBS" 2>&1 | grep -v "^make\|^\\[" || true

    if [ ! -f "$BUILD_DIR/pixel" ]; then
        print_error "Build failed"
        exit 1
    fi

    print_success "Build complete"
}

# Install Pixel binary
install_pixel() {
    # Determine install location
    if [ "$EUID" -eq 0 ]; then
        INSTALL_DIR="/usr/local/bin"
    else
        INSTALL_DIR="$HOME/.local/bin"
    fi

    print_step "Installing to $INSTALL_DIR..."

    mkdir -p "$INSTALL_DIR"
    cp "$BUILD_DIR/pixel" "$INSTALL_DIR/pixel"
    chmod +x "$INSTALL_DIR/pixel"

    print_success "Installed successfully"
}

# Setup PATH if needed
setup_path() {
    if [ "$EUID" -eq 0 ]; then
        return  # /usr/local/bin is usually in PATH
    fi

    INSTALL_DIR="$HOME/.local/bin"

    if [[ ":$PATH:" != *":$INSTALL_DIR:"* ]]; then
        SHELL_NAME=$(basename "$SHELL")
        SHELL_RC=""

        case "$SHELL_NAME" in
            zsh)  SHELL_RC="$HOME/.zshrc" ;;
            bash) SHELL_RC="$HOME/.bashrc" ;;
            fish) SHELL_RC="$HOME/.config/fish/config.fish" ;;
        esac

        if [ -n "$SHELL_RC" ]; then
            # Check if already added
            if ! grep -q "/.local/bin" "$SHELL_RC" 2>/dev/null; then
                print_step "Adding $INSTALL_DIR to PATH..."

                if [ "$SHELL_NAME" = "fish" ]; then
                    echo "fish_add_path $INSTALL_DIR" >> "$SHELL_RC"
                else
                    echo 'export PATH="$HOME/.local/bin:$PATH"' >> "$SHELL_RC"
                fi

                print_success "Added to $SHELL_RC"
                echo ""
                print_warning "Run 'source $SHELL_RC' or restart your terminal to use 'pixel'"
            fi
        fi
    fi
}

# Main installation flow
main() {
    echo ""
    echo "  ____  _          _ "
    echo " |  _ \\(_)_  _____| |"
    echo " | |_) | \\ \\/ / _ \\ |"
    echo " |  __/| |>  <  __/ |"
    echo " |_|   |_/_/\\_\\___|_|"
    echo ""
    echo " Pixel Programming Language Installer"
    echo ""

    detect_os
    install_dependencies
    setup_repository
    build_pixel
    install_pixel
    setup_path

    echo ""
    print_success "Installation complete!"
    echo ""
    echo "  Run a game:   pixel game.pixel"
    echo "  Get help:     pixel help"
    echo ""
}

main
