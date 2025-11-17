ting ($BUILD_TYPE) ---"
    cd "$BUILD_DIR" && ctest --output-on-failure

    echo "--- Verify CLI ($BUILD_TYPE) ---"
    "$BUILD_DIR/pixel" version

    echo ""
done

# Cleanup
rm -rf "$BUILD_DIR"

echo "=== All CI checks passed ==="
"$PROJECT_DIR/build-ci"

echo "=== Local CI Check ==="
echo ""

# Clean build directory
r#!/bin/bash
set -e

# Run the same checks as CI locally before pushing

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR=