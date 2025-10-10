ack a 32-bit color value into RGBA components
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
 and input

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
#defin