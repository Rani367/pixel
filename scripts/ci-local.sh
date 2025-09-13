ting ($BUILD_TYPE) ---"
    cd "$BUILD_DIR" && ctest --output-on-failure

    echo "--- Verify CLI ($BUILD_TYPE) ---"
    "$BUILD_DIR/pixel" version

    echo ""
done

# Cleanup
rm -rf "$BUILD_DIR"

echo "=== All CI checks passed ==="
