// Engine Native Functions Implementation

#include "engine/engine_natives.h"
#include "engine/engine.h"
#include "engine/physics.h"
#include "runtime/stdlib.h"
#include "vm/object.h"
#include "pal/pal.h"
#include <stdio.h>
#include <string.h>

// ============================================================================
// Helper Functions
// ============================================================================

// Define a global constant value
static void define_constant(VM* vm, const char* name, Value value) {
    ObjString* name_str = string_copy(name, (int)strlen(name));
    vm_define_global(vm, name_str, value);
}

// Helper for runtime errors from native functions
static Value native_error(const char* message) {
    fprintf(stderr, "Runtime error: %s\n", message);
    return NONE_VAL;
}

// Helper to apply camera transform to world coordinates
static void apply_camera_transform(double world_x, double world_y, int* screen_x, int* screen_y) {
    Engine* engine = engine_get();
    if (!engine) {
        *screen_x = (int)world_x;
        *screen_y = (int)world_y;
        return;
    }

    if (engine->camera) {
        ObjCamera* cam = engine->camera;
        int width = engine_get_width(engine);
        int height = engine_get_height(engine);

        // Transform: (world - camera_pos) * zoom + screen_center
        *screen_x = (int)((world_x - cam->x - cam->shake_offset_x) * cam->zoom + width / 2.0);
        *screen_y = (int)((world_y - cam->y - cam->shake_offset_y) * cam->zoom + height / 2.0);
    } else {
        *screen_x = (int)world_x;
        *screen_y = (int)world_y;
    }
}

// Helper to apply camera zoom to a dimension
static int apply_camera_zoom(int dimension) {
    Engine* engine = engine_get();
    if (engine && engine->camera) {
        return (int)(dimension * engine->camera->zoom);
    }
    return dimension;
}

// ============================================================================
// Color Functions
// ============================================================================

// rgb(r, g, b) -> color
static Value native_rgb(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_NUMBER(args[0]) || !IS_NUMBER(args[1]) || !IS_NUMBER(args[2])) {
        return native_error("rgb() requires three numbers");
    }

    int r = (int)AS_NUMBER(args[0]);
    int g = (int)AS_NUMBER(args[1]);
    int b = (int)AS_NUMBER(args[2]);

    // Clamp to 0-255
    if (r < 0) r = 0;
    if (r > 255) r = 255;
    if (g < 0) g = 0;
    if (g > 255) g = 255;
    if (b < 0) b = 0;
    if (b > 255) b = 255;

    uint32_t color = pack_color((uint8_t)r, (uint8_t)g, (uint8_t)b, 255);
    return NUMBER_VAL((double)color);
}

// rgba(r, g, b, a) -> color
static Value native_rgba(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_NUMBER(args[0]) || !IS_NUMBER(args[1]) ||
        !IS_NUMBER(args[2]) || !IS_NUMBER(args[3])) {
        return native_error("rgba() requires four numbers");
    }

    int r = (int)AS_NUMBER(args[0]);
    int g = (int)AS_NUMBER(args[1]);
    int b = (int)AS_NUMBER(args[2]);
    int a = (int)AS_NUMBER(args[3]);

    // Clamp to 0-255
    if (r < 0) r = 0;
    if (r > 255) r = 255;
    if (g < 0) g = 0;
    if (g > 255) g = 255;
    if (b < 0) b = 0;
    if (b > 255) b = 255;
    if (a < 0) a = 0;
    if (a > 255) a = 255;

    uint32_t color = pack_color((uint8_t)r, (uint8_t)g, (uint8_t)b, (uint8_t)a);
    return NUMBER_VAL((double)color);
}

// ============================================================================
// Window Functions
// ============================================================================

// create_window(width?, height?, title?) -> nil
static Value native_create_window(int arg_count, Value* args) {
    Engine* engine = engine_get();
    if (!engine) {
        return native_error("No engine initialized");
    }

    int width = ENGINE_DEFAULT_WIDTH;
    int height = ENGINE_DEFAULT_HEIGHT;
    const char* title = ENGINE_DEFAULT_TITLE;

    if (arg_count >= 1 && IS_NUMBER(args[0])) {
        width = (int)AS_NUMBER(args[0]);
    }
    if (arg_count >= 2 && IS_NUMBER(args[1])) {
        height = (int)AS_NUMBER(args[1]);
    }
    if (arg_count >= 3 && IS_STRING(args[2])) {
        title = AS_CSTRING(args[2]);
    }

    engine_create_window(engine, title, width, height);
    return NONE_VAL;
}

// set_title(title) -> nil
static Value native_set_title(int arg_count, Value* args) {
    (void)arg_count;

    Engine* engine = engine_get();
    if (!engine) {
        return native_error("No engine initialized");
    }

    if (!IS_STRING(args[0])) {
        return native_error("set_title() requires a string");
    }

    engine_set_title(engine, AS_CSTRING(args[0]));
    return NONE_VAL;
}

// window_width() -> number
static Value native_window_width(int arg_count, Value* args) {
    (void)arg_count;
    (void)args;

    Engine* engine = engine_get();
    if (!engine) {
        return NUMBER_VAL(0);
    }

    return NUMBER_VAL((double)engine_get_width(engine));
}

// window_height() -> number
static Value native_window_height(int arg_count, Value* args) {
    (void)arg_count;
    (void)args;

    Engine* engine = engine_get();
    if (!engine) {
        return NUMBER_VAL(0);
    }

    return NUMBER_VAL((double)engine_get_height(engine));
}

// ============================================================================
// Drawing Functions
// ============================================================================

// clear(color) -> nil
static Value native_clear(int arg_count, Value* args) {
    (void)arg_count;

    Engine* engine = engine_get();
    if (!engine || !engine->window) {
        return NONE_VAL;
    }

    if (!IS_NUMBER(args[0])) {
        return native_error("clear() requires a color");
    }

    uint32_t color = (uint32_t)AS_NUMBER(args[0]);
    uint8_t r, g, b, a;
    unpack_color(color, &r, &g, &b, &a);
    (void)a;  // Alpha not used for clear

    pal_window_clear(engine->window, r, g, b);
    return NONE_VAL;
}

// draw_rect(x, y, width, height, color) -> nil
static Value native_draw_rect(int arg_count, Value* args) {
    (void)arg_count;

    Engine* engine = engine_get();
    if (!engine || !engine->window) {
        return NONE_VAL;
    }

    if (!IS_NUMBER(args[0]) || !IS_NUMBER(args[1]) ||
        !IS_NUMBER(args[2]) || !IS_NUMBER(args[3]) ||
        !IS_NUMBER(args[4])) {
        return native_error("draw_rect() requires x, y, width, height, color");
    }

    double world_x = AS_NUMBER(args[0]);
    double world_y = AS_NUMBER(args[1]);
    int w = (int)AS_NUMBER(args[2]);
    int h = (int)AS_NUMBER(args[3]);
    uint32_t color = (uint32_t)AS_NUMBER(args[4]);

    // Apply camera transform
    int screen_x, screen_y;
    apply_camera_transform(world_x, world_y, &screen_x, &screen_y);
    w = apply_camera_zoom(w);
    h = apply_camera_zoom(h);

    uint8_t r, g, b, a;
    unpack_color(color, &r, &g, &b, &a);

    pal_draw_rect(engine->window, screen_x, screen_y, w, h, r, g, b, a);
    return NONE_VAL;
}

// draw_circle(x, y, radius, color) -> nil
static Value native_draw_circle(int arg_count, Value* args) {
    (void)arg_count;

    Engine* engine = engine_get();
    if (!engine || !engine->window) {
        return NONE_VAL;
    }

    if (!IS_NUMBER(args[0]) || !IS_NUMBER(args[1]) ||
        !IS_NUMBER(args[2]) || !IS_NUMBER(args[3])) {
        return native_error("draw_circle() requires x, y, radius, color");
    }

    double world_x = AS_NUMBER(args[0]);
    double world_y = AS_NUMBER(args[1]);
    int radius = (int)AS_NUMBER(args[2]);
    uint32_t color = (uint32_t)AS_NUMBER(args[3]);

    // Apply camera transform
    int screen_x, screen_y;
    apply_camera_transform(world_x, world_y, &screen_x, &screen_y);
    radius = apply_camera_zoom(radius);

    uint8_t r, g, b, a;
    unpack_color(color, &r, &g, &b, &a);

    pal_draw_circle(engine->window, screen_x, screen_y, radius, r, g, b, a);
    return NONE_VAL;
}

// draw_line(x1, y1, x2, y2, color) -> nil
static Value native_draw_line(int arg_count, Value* args) {
    (void)arg_count;

    Engine* engine = engine_get();
    if (!engine || !engine->window) {
        return NONE_VAL;
    }

    if (!IS_NUMBER(args[0]) || !IS_NUMBER(args[1]) ||
        !IS_NUMBER(args[2]) || !IS_NUMBER(args[3]) ||
        !IS_NUMBER(args[4])) {
        return native_error("draw_line() requires x1, y1, x2, y2, color");
    }

    double world_x1 = AS_NUMBER(args[0]);
    double world_y1 = AS_NUMBER(args[1]);
    double world_x2 = AS_NUMBER(args[2]);
    double world_y2 = AS_NUMBER(args[3]);
    uint32_t color = (uint32_t)AS_NUMBER(args[4]);

    // Apply camera transform to both endpoints
    int screen_x1, screen_y1, screen_x2, screen_y2;
    apply_camera_transform(world_x1, world_y1, &screen_x1, &screen_y1);
    apply_camera_transform(world_x2, world_y2, &screen_x2, &screen_y2);

    uint8_t r, g, b, a;
    unpack_color(color, &r, &g, &b, &a);

    pal_draw_line(engine->window, screen_x1, screen_y1, screen_x2, screen_y2, r, g, b, a);
    return NONE_VAL;
}

// ============================================================================
// Input Functions
// ============================================================================

// key_down(key_code) -> bool
static Value native_key_down(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_NUMBER(args[0])) {
        return native_error("key_down() requires a key code");
    }

    int key = (int)AS_NUMBER(args[0]);
    return BOOL_VAL(pal_key_down((PalKey)key));
}

// key_pressed(key_code) -> bool
static Value native_key_pressed(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_NUMBER(args[0])) {
        return native_error("key_pressed() requires a key code");
    }

    int key = (int)AS_NUMBER(args[0]);
    return BOOL_VAL(pal_key_pressed((PalKey)key));
}

// mouse_x() -> number
static Value native_mouse_x(int arg_count, Value* args) {
    (void)arg_count;
    (void)args;

    int x = 0, y = 0;
    pal_mouse_position(&x, &y);
    return NUMBER_VAL((double)x);
}

// mouse_y() -> number
static Value native_mouse_y(int arg_count, Value* args) {
    (void)arg_count;
    (void)args;

    int x = 0, y = 0;
    pal_mouse_position(&x, &y);
    return NUMBER_VAL((double)y);
}

// mouse_down(button) -> bool
static Value native_mouse_down(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_NUMBER(args[0])) {
        return native_error("mouse_down() requires a button number");
    }

    int button = (int)AS_NUMBER(args[0]);
    return BOOL_VAL(pal_mouse_down((PalMouseButton)button));
}

// key_released(key_code) -> bool
static Value native_key_released(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_NUMBER(args[0])) {
        return native_error("key_released() requires a key code");
    }

    int key = (int)AS_NUMBER(args[0]);
    return BOOL_VAL(pal_key_released((PalKey)key));
}

// mouse_pressed(button) -> bool
static Value native_mouse_pressed(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_NUMBER(args[0])) {
        return native_error("mouse_pressed() requires a button number");
    }

    int button = (int)AS_NUMBER(args[0]);
    return BOOL_VAL(pal_mouse_pressed((PalMouseButton)button));
}

// mouse_released(button) -> bool
static Value native_mouse_released(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_NUMBER(args[0])) {
        return native_error("m