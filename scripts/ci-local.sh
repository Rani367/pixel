#!/bin/bash
set -e

# Run the same checks as CI locally before pushing

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_DIR/build-ci"

echo "=== Local CI Check ==="
echo ""

# Clean build directory
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"

for BUILD_TYPE in Debug Release; do
    echo "--- Building ($BUILD_TYPE) ---"
    cmake -B "$BUILD_DIR" -S "$PROJECT_DIR" -DCMAKE_BUILD_TYPE=$BUILD_TYPE
    cmake --build "$BUILD_DIR" --clean-first

    echo "--- Testing ($BUILD_TYPE) ---"
    cd "$BUILD_DIR" && ctest --output-on-failure

    echo "--- Verify CLI ($BUILD_TYPE) ---"
    "$BUILD_DIR/pixel" version

    echo ""
done

# Cleanup
rm -rf "$BUILD_DIR"

echo "=== All CI checks passed ==="
