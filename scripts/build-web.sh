#!/bin/bash
# Build Pixel for the web using Emscripten
#
# Prerequisites:
#   - Emscripten SDK installed and activdirectory: $BUILD_DIR"

# Configure wiated (source emsdk_env.sh)
#
# Usage:
#   ./scripts/build-web.sh [Release|Debug]
#
# Output:
#   build-web/pixel.html  - Main HTML file to open in browser
#   build-web/pixel.js    - JavaScript glue code
#   build-web/pixel.wasm  - WebAssembly binary

set -e

# Build type (default: Release)
BUILD_TYPE="${1:-Release}"

# Check if Emscripten is available
if ! command -v emcmake &> /dev/null; then
    echo "Error: E