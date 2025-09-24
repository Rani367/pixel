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
        return native_error("m 3);
    define_native(vm, "lerp", native_lerp, 3);
    define_native(vm, "lerp_angle", native_lerp_angle, 3);

    // Camera functions
    define_native(vm, "camera", native_camera, 0);
    define_native(vm, "camera_x", native_camera_x, 0);
    define_native(vm, "camera_y", native_camera_y, 0);
    define_native(vm, "camera_zoom", native_camera_zoom, 0);
    define_native(vm, "camera_set_position", native_camera_set_position, 2);
    define_native(vm, "camera_set_zoom", native_camera_set_zoom, 1);
    define_native(vm, "camera_follow", native_camera_follow, -1);  // 1-2 args
    define_native(vm, "camera_shake", native_camera_shake, 2);
    define_native(vm, "screen_to_world_x", native_screen_to_world_x, 1);
    define_native(vm, "screen_to_world_y", native_screen_to_world_y, 1);
    define_native(vm, "world_to_screen_x", native_world_to_screen_x, 1);
    define_native(vm, "world_to_screen_y", native_world_to_screen_y, 1);

    // Animation functions
    define_native(vm, "create_animation", native_create_animation, 5);
    define_native(vm, "animation_play", native_animation_play, 1);
    define_native(vm, "animation_stop", native_animation_stop, 1);
    define_native(vm, "animation_reset", native_animation_reset, 1);
    define_native(vm, "animation_set_looping", native_animation_set_looping, 2);
    define_native(vm, "animation_frame", native_animation_frame, 1);
    define_native(vm, "animation_playing", native_animation_playing, 1);
    define_native(vm, "sprite_set_animation", native_sprite_set_animation, 2);
    define_native(vm, "sprite_play", native_sprite_play, 1);
    define_native(vm, "sprite_stop", native_sprite_stop, 1);

    // Scene functions
    define_native(vm, "load_scene", native_load_scene, 1);
    define_native(vm, "get_scene", native_get_scene, 0);

    // Particle functions
    define_native(vm, "create_emitter", native_create_emitter, 2);
    define_native(vm, "emitter_emit", native_emitter_emit, 2);
    define_native(vm, "emitter_set_color", native_emitter_set_color, 2);
    define_native(vm, "emitter_set_speed", native_emitter_set_speed, 3);
    define_native(vm, "emitter_set_angle", native_emitter_set_angle, 3);
    define_native(vm, "emitter_set_lifetime", native_emitter_set_lifetime, 3);
    define_native(vm, "emitter_set_size", native_emitter_set_size, 3);
    define_native(vm, "emitter_set_gravity", native_emitter_set_gravity, 2);
    define_native(vm, "emitter_set_rate", native_emitter_set_rate, 2);
    define_native(vm, "emitter_set_position", native_emitter_set_position, 3);
    define_native(vm, "emitter_set_active", native_emitter_set_active, 2);
    define_native(vm, "emitter_count", native_emitter_count, 1);
    define_native(vm, "draw_particles", native_draw_particles, 1);

    // Color constants
    define_constant(vm, "RED", NUMBER_VAL((double)COLOR_RED));
    define_constant(vm, "GREEN", NUMBER_VAL((double)COLOR_GREEN));
    define_constant(vm, "BLUE", NUMBER_VAL((double)COLOR_BLUE));
    define_constant(vm, "WHITE", NUMBER_VAL((double)COLOR_WHITE));
    define_constant(vm, "BLACK", NUMBER_VAL((double)COLOR_BLACK));
    define_constant(vm, "YELLOW", NUMBER_VAL((double)COLOR_YELLOW));
    define_constant(vm, "CYAN", NUMBER_VAL((double)COLOR_CYAN));
    define_constant(vm, "MAGENTA", NUMBER_VAL((double)COLOR_MAGENTA));
    define_constant(vm, "ORANGE", NUMBER_VAL((double)COLOR_ORANGE));
    define_constant(vm, "PURPLE", NUMBER_VAL((double)COLOR_PURPLE));
    define_constant(vm, "GRAY", NUMBER_VAL((double)COLOR_GRAY));
    define_constant(vm, "GREY", NUMBER_VAL((double)COLOR_GREY));

    // Key constants
    define_constant(vm, "KEY_UP", NUMBER_VAL((double)PAL_KEY_UP));
    define_constant(vm, "KEY_DOWN", NUMBER_VAL((double)PAL_KEY_DOWN));
    define_constant(vm, "KEY_LEFT", NUMBER_VAL((double)PAL_KEY_LEFT));
    define_constant(vm, "KEY_RIGHT", NUMBER_VAL((double)PAL_KEY_RIGHT));
    define_constant(vm, "KEY_SPACE", NUMBER_VAL((double)PAL_KEY_SPACE));
    define_constant(vm, "KEY_RETURN", NUMBER_VAL((double)PAL_KEY_RETURN));
    define_constant(vm, "KEY_ESCAPE", NUMBER_VAL((double)PAL_KEY_ESCAPE));
    define_constant(vm, "KEY_TAB", NUMBER_VAL((double)PAL_KEY_TAB));

    // Letter keys
    define_constant(vm, "KEY_A", NUMBER_VAL((double)PAL_KEY_A));
    define_constant(vm, "KEY_B", NUMBER_VAL((double)PAL_KEY_B));
    define_constant(vm, "KEY_C", NUMBER_VAL((double)PAL_KEY_C));
    define_constant(vm, "KEY_D", NUMBER_VAL((double)PAL_KEY_D));
    define_constant(vm, "KEY_E", NUMBER_VAL((double)PAL_KEY_E));
    define_constant(vm, "KEY_F", NUMBER_VAL((double)PAL_KEY_F));
    define_constant(vm, "KEY_G", NUMBER_VAL((double)PAL_KEY_G));
    define_constant(vm, "KEY_H", NUMBER_VAL((double)PAL_KEY_H));
    define_constant(vm, "KEY_I", NUMBER_VAL((double)PAL_KEY_I));
    define_constant(vm, "KEY_J", NUMBER_VAL((double)PAL_KEY_J));
    define_constant(vm, "KEY_K", NUMBER_VAL((double)PAL_KEY_K));
    define_constant(vm, "KEY_L", NUMBER_VAL((double)PAL_KEY_L));
    define_constant(vm, "KEY_M", NUMBER_VAL((double)PAL_KEY_M));
    define_constant(vm, "KEY_N", NUMBER_VAL((double)PAL_KEY_N));
    define_constant(vm, "KEY_O", NUMBER_VAL((double)PAL_KEY_O));
    define_constant(vm, "KEY_P", NUMBER_VAL((double)PAL_KEY_P));
    define_constant(vm, "KEY_Q", NUMBER_VAL((double)PAL_KEY_Q));
    define_constant(vm, "KEY_R", NUMBER_VAL((double)PAL_KEY_R));
    define_constant(vm, "KEY_S", NUMBER_VAL((double)PAL_KEY_S));
    define_constant(vm, "KEY_T", NUMBER_VAL((double)PAL_KEY_T));
    define_constant(vm, "KEY_U", NUMBER_VAL((double)PAL_KEY_U));
    define_constant(vm, "KEY_V", NUMBER_VAL((double)PAL_KEY_V));
    define_constant(vm, "KEY_W", NUMBER_VAL((double)PAL_KEY_W));
    define_constant(vm, "KEY_X", NUMBER_VAL((double)PAL_KEY_X));
    define_constant(vm, "KEY_Y", NUMBER_VAL((double)PAL_KEY_Y));
    define_constant(vm, "KEY_Z", NUMBER_VAL((double)PAL_KEY_Z));

    // Number keys
    define_constant(vm, "KEY_0", NUMBER_VAL((double)PAL_KEY_0));
    define_constant(vm, "KEY_1", NUMBER_VAL((double)PAL_KEY_1));
    define_constant(vm, "KEY_2", NUMBER_VAL((double)PAL_KEY_2));
    define_constant(vm, "KEY_3", NUMBER_VAL((double)PAL_KEY_3));
    define_constant(vm, "KEY_4", NUMBER_VAL((double)PAL_KEY_4));
    define_constant(vm, "KEY_5", NUMBER_VAL((double)PAL_KEY_5));
    define_constant(vm, "KEY_6", NUMBER_VAL((double)PAL_KEY_6));
    define_constant(vm, "KEY_7", NUMBER_VAL((double)PAL_KEY_7));
    define_constant(vm, "KEY_8", NUMBER_VAL((double)PAL_KEY_8));
    define_constant(vm, "KEY_9", NUMBER_VAL((double)PAL_KEY_9));

    // Modifier keys (use left variants as the default)
    define_constant(vm, "KEY_SHIFT", NUMBER_VAL((double)PAL_KEY_LSHIFT));
    define_constant(vm, "KEY_CTRL", NUMBER_VAL((double)PAL_KEY_LCTRL));
    define_constant(vm, "KEY_ALT", NUMBER_VAL((double)PAL_KEY_LALT));
    define_constant(vm, "KEY_LSHIFT", NUMBER_VAL((double)PAL_KEY_LSHIFT));
    define_constant(vm, "KEY_RSHIFT", NUMBER_VAL((double)PAL_KEY_RSHIFT));
    define_constant(vm, "KEY_LCTRL", NUMBER_VAL((double)PAL_KEY_LCTRL));
    define_constant(vm, "KEY_RCTRL", NUMBER_VAL((double)PAL_KEY_RCTRL));
    define_constant(vm, "KEY_LALT", NUMBER_VAL((double)PAL_KEY_LALT));
    define_constant(vm, "KEY_RALT", NUMBER_VAL((double)PAL_KEY_RALT));

    // Function keys
    define_constant(vm, "KEY_F1", NUMBER_VAL((double)PAL_KEY_F1));
    define_constant(vm, "KEY_F2", NUMBER_VAL((double)PAL_KEY_F2));
    define_constant(vm, "KEY_F3", NUMBER_VAL((double)PAL_KEY_F3));
    define_constant(vm, "KEY_F4", NUMBER_VAL((double)PAL_KEY_F4));
    define_constant(vm, "KEY_F5", NUMBER_VAL((double)PAL_KEY_F5));
    define_constant(vm, "KEY_F6", NUMBER_VAL((double)PAL_KEY_F6));
    define_constant(vm, "KEY_F7", NUMBER_VAL((double)PAL_KEY_F7));
    define_constant(vm, "KEY_F8", NUMBER_VAL((double)PAL_KEY_F8));
    define_constant(vm, "KEY_F9", NUMBER_VAL((double)PAL_KEY_F9));
    define_constant(vm, "KEY_F10", NUMBER_VAL((double)PAL_KEY_F10));
    define_constant(vm, "KEY_F11", NUMBER_VAL((double)PAL_KEY_F11));
    define_constant(vm, "KEY_F12", NUMBER_VAL((double)PAL_KEY_F12));

    // Additional special keys
    define_constant(vm, "KEY_BACKSPACE", NUMBER_VAL((double)PAL_KEY_BACKSPACE));

    // Mouse button constants
    define_constant(vm, "MOUSE_LEFT", NUMBER_VAL((double)PAL_MOUSE_LEFT));
    define_constant(vm, "MOUSE_MIDDLE", NUMBER_VAL((double)PAL_MOUSE_MIDDLE));
    define_constant(vm, "MOUSE_RIGHT", NUMBER_VAL((double)PAL_MOUSE_RIGHT));
}
