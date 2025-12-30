// Engine Native Functions
// Provides native functions for window management, drawing, colors, and input

#ifndef PH_ENGINE_NATIVES_H
#define PH_ENGINE_NATIVES_H

#include "vm/vm.h"
#include "vm/value.h"
#include <stdint.h>

// ============================================================================
// Color Constants (packed as 0xRRGGBBAA)
// ============================================================================

#define COLOR_RED       0xFF0000FFu
#define COLOR_GREEN     0x00FF00FFu
#define COLOR_BLUE      0x0000FFFFu
#define COLOR_WHITE     0xFFFFFFFFu
#define COLOR_BLACK     0x000000FFu
#define COLOR_YELLOW    0xFFFF00FFu
#define COLOR_CYAN      0x00FFFFFFu
#define COLOR_MAGENTA   0xFF00FFFFu
#define COLOR_ORANGE    0xFF8800FFu
#define COLOR_PURPLE    0x8800FFFFu
#define COLOR_GRAY      0x808080FFu
#define COLOR_GREY      0x808080FFu  // British spelling alias

// ============================================================================
// Color Utility Functions
// ============================================================================

// Pack RGBA components into a 32-bit color value
static inline uint32_t pack_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    return ((uint32_t)r << 24) | ((uint32_t)g << 16) | ((uint32_t)b << 8) | a;
}

// Unpack a 32-bit color value into RGBA components
static inline void unpack_color(uint32_t color, uint8_t* r, uint8_t* g, uint8_t* b, uint8_t* a) {
    *r = (uint8_t)((color >> 24) & 0xFF);
    *g = (uint8_t)((color >> 16) & 0xFF);
    *b = (uint8_t)((color >> 8) & 0xFF);
    *a = (uint8_t)(color & 0xFF);
}

// ============================================================================
// Native Function Registration
// ============================================================================

// Register all engine native functions with the VM
void engine_natives_init(VM* vm);

#endif // PH_ENGINE_NATIVES_H
