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
    echo "Error: Eth Emscripten
cd "$BUILD_DIR"
emcmake cmake "$PROJECT_ROOT" \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DCMAKE_VERBOSE_MAKEFILE=OFF

# Build
emmake make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

echo ""
echo "Build complete!"
echo ""
echo "Output files:"
ls -la pixel.html pixel.js pixel.wasm 2>/dev/null || true
echo ""
echo "To test locally, start a web server:"
echo "  cd $BUILD_DIR"
echo "  python3 -m http.server 8000"
echo ""
echo "Then open: http://localhost:8000/pixel.html"
