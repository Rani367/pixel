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

// LCOV_EXCL_START - camera transform helpers require full engine/camera setup
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
// LCOV_EXCL_STOP

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
        return native_error("No engine initialized");  // LCOV_EXCL_LINE
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
        return native_error("No engine initialized");  // LCOV_EXCL_LINE
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
        return NUMBER_VAL(0);  // LCOV_EXCL_LINE
    }

    return NUMBER_VAL((double)engine_get_width(engine));
}

// window_height() -> number
static Value native_window_height(int arg_count, Value* args) {
    (void)arg_count;
    (void)args;

    Engine* engine = engine_get();
    if (!engine) {
        return NUMBER_VAL(0);  // LCOV_EXCL_LINE
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
        return NONE_VAL;  // LCOV_EXCL_LINE
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
        return NONE_VAL;  // LCOV_EXCL_LINE
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
        return NONE_VAL;  // LCOV_EXCL_LINE
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
        return NONE_VAL;  // LCOV_EXCL_LINE
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
        return native_error("mouse_released() requires a button number");
    }

    int button = (int)AS_NUMBER(args[0]);
    return BOOL_VAL(pal_mouse_released((PalMouseButton)button));
}

// ============================================================================
// Image and Sprite Functions
// ============================================================================

// load_image(path) -> image
static Value native_load_image(int arg_count, Value* args) {
    (void)arg_count;

    Engine* engine = engine_get();
    // LCOV_EXCL_START - requires window/file
    if (!engine || !engine->window) {
        return native_error("No window created. Call create_window() first");
    }
    // LCOV_EXCL_STOP

    if (!IS_STRING(args[0])) {
        return native_error("load_image() requires a string path");
    }

    const char* path = AS_CSTRING(args[0]);
    PalTexture* texture = pal_texture_load(engine->window, path);
    if (!texture) {
        return native_error("Failed to load image");  // LCOV_EXCL_LINE
    }

    int width = 0, height = 0;
    pal_texture_get_size(texture, &width, &height);

    ObjString* path_str = string_copy(path, (int)strlen(path));
    ObjImage* image = image_new(texture, width, height, path_str);
    return OBJECT_VAL(image);
}

// image_width(image) -> number
static Value native_image_width(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_IMAGE(args[0])) {
        return native_error("image_width() requires an image");
    }

    ObjImage* image = AS_IMAGE(args[0]);
    return NUMBER_VAL((double)image->width);
}

// image_height(image) -> number
static Value native_image_height(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_IMAGE(args[0])) {
        return native_error("image_height() requires an image");
    }

    ObjImage* image = AS_IMAGE(args[0]);
    return NUMBER_VAL((double)image->height);
}

// draw_image(image, x, y) -> nil
static Value native_draw_image(int arg_count, Value* args) {
    (void)arg_count;

    Engine* engine = engine_get();
    if (!engine || !engine->window) {
        return NONE_VAL;  // LCOV_EXCL_LINE
    }

    // LCOV_EXCL_START - type validation errors
    if (!IS_IMAGE(args[0])) {
        return native_error("draw_image() requires an image as first argument");
    }
    if (!IS_NUMBER(args[1]) || !IS_NUMBER(args[2])) {
        return native_error("draw_image() requires x and y as numbers");
    }
    // LCOV_EXCL_STOP

    ObjImage* image = AS_IMAGE(args[0]);
    double world_x = AS_NUMBER(args[1]);
    double world_y = AS_NUMBER(args[2]);

    // Apply camera transform
    int screen_x, screen_y;
    apply_camera_transform(world_x, world_y, &screen_x, &screen_y);
    int width = apply_camera_zoom(image->width);
    int height = apply_camera_zoom(image->height);

    if (image->texture) {
        pal_draw_texture(engine->window, image->texture, screen_x, screen_y,
                         width, height);
    }

    return NONE_VAL;
}

// draw_image_ex(image, x, y, width, height, rotation, flip_x, flip_y) -> nil
static Value native_draw_image_ex(int arg_count, Value* args) {
    (void)arg_count;

    Engine* engine = engine_get();
    if (!engine || !engine->window) {
        return NONE_VAL;  // LCOV_EXCL_LINE
    }

    // LCOV_EXCL_START - type validation errors
    if (!IS_IMAGE(args[0])) {
        return native_error("draw_image_ex() requires an image as first argument");
    }
    if (!IS_NUMBER(args[1]) || !IS_NUMBER(args[2]) ||
        !IS_NUMBER(args[3]) || !IS_NUMBER(args[4]) ||
        !IS_NUMBER(args[5])) {
        return native_error("draw_image_ex() requires x, y, width, height, rotation as numbers");
    }
    // LCOV_EXCL_STOP

    ObjImage* image = AS_IMAGE(args[0]);
    double world_x = AS_NUMBER(args[1]);
    double world_y = AS_NUMBER(args[2]);
    int width = (int)AS_NUMBER(args[3]);
    int height = (int)AS_NUMBER(args[4]);
    double rotation = AS_NUMBER(args[5]);
    bool flip_x = IS_BOOL(args[6]) ? AS_BOOL(args[6]) : false;
    bool flip_y = IS_BOOL(args[7]) ? AS_BOOL(args[7]) : false;

    // Apply camera transform
    int screen_x, screen_y;
    apply_camera_transform(world_x, world_y, &screen_x, &screen_y);
    width = apply_camera_zoom(width);
    height = apply_camera_zoom(height);

    if (image->texture) {
        // Origin at top-left for this function
        pal_draw_texture_ex(engine->window, image->texture, screen_x, screen_y, width, height,
                            rotation, 0, 0, flip_x, flip_y);
    }

    return NONE_VAL;
}

// create_sprite(image) -> sprite
static Value native_create_sprite(int arg_count, Value* args) {
    (void)arg_count;

    ObjImage* image = NULL;
    if (arg_count >= 1 && IS_IMAGE(args[0])) {
        image = AS_IMAGE(args[0]);
    } else if (arg_count >= 1 && !IS_NONE(args[0])) {
        return native_error("create_sprite() requires an image or none");
    }

    ObjSprite* sprite = sprite_new(image);
    return OBJECT_VAL(sprite);
}

// LCOV_EXCL_START - sprite rendering requires full engine/window setup
// draw_sprite(sprite) -> nil
static Value native_draw_sprite(int arg_count, Value* args) {
    (void)arg_count;

    Engine* engine = engine_get();
    if (!engine || !engine->window) {
        return NONE_VAL;
    }

    if (!IS_SPRITE(args[0])) {
        return native_error("draw_sprite() requires a sprite");
    }

    ObjSprite* sprite = AS_SPRITE(args[0]);

    // Don't draw if not visible or no image
    if (!sprite->visible || !sprite->image || !sprite->image->texture) {
        return NONE_VAL;
    }

    ObjImage* image = sprite->image;

    // Calculate final dimensions
    double width = sprite->width > 0 ? sprite->width : image->width;
    double height = sprite->height > 0 ? sprite->height : image->height;
    width *= sprite->scale_x;
    height *= sprite->scale_y;

    // Apply camera zoom to dimensions
    double cam_zoom = 1.0;
    if (engine->camera) {
        cam_zoom = engine->camera->zoom;
    }
    double scaled_width = width * cam_zoom;
    double scaled_height = height * cam_zoom;

    // Calculate origin in pixels (scaled)
    int origin_x = (int)(sprite->origin_x * scaled_width);
    int origin_y = (int)(sprite->origin_y * scaled_height);

    // Transform sprite world position to screen position
    int screen_x, screen_y;
    apply_camera_transform(sprite->x, sprite->y, &screen_x, &screen_y);

    // Calculate position (adjust for origin)
    int draw_x = screen_x - origin_x;
    int draw_y = screen_y - origin_y;

    // Check if using sprite sheet frames
    if (sprite->frame_width > 0 && sprite->frame_height > 0) {
        // Draw sprite sheet frame
        pal_draw_texture_region(engine->window, image->texture,
                                sprite->frame_x, sprite->frame_y,
                                sprite->frame_width, sprite->frame_height,
                                draw_x, draw_y, (int)scaled_width, (int)scaled_height);
    } else if (sprite->rotation != 0 || sprite->flip_x || sprite->flip_y) {
        // Draw with rotation/flip
        pal_draw_texture_ex(engine->window, image->texture,
                            screen_x, screen_y, (int)scaled_width, (int)scaled_height,
                            sprite->rotation, origin_x, origin_y,
                            sprite->flip_x, sprite->flip_y);
    } else {
        // Simple draw
        pal_draw_texture(engine->window, image->texture, draw_x, draw_y,
                         (int)scaled_width, (int)scaled_height);
    }

    return NONE_VAL;
}
// LCOV_EXCL_STOP

// LCOV_EXCL_START - sprite frame functions require full sprite setup
// set_sprite_frame(sprite, frame_index) -> nil
// Calculates frame_x and frame_y based on frame_width, frame_height, and image dimensions
static Value native_set_sprite_frame(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_SPRITE(args[0])) {
        return native_error("set_sprite_frame() requires a sprite as first argument");
    }
    if (!IS_NUMBER(args[1])) {
        return native_error("set_sprite_frame() requires frame index as number");
    }

    ObjSprite* sprite = AS_SPRITE(args[0]);
    int frame_index = (int)AS_NUMBER(args[1]);

    // Must have frame dimensions set
    if (sprite->frame_width <= 0 || sprite->frame_height <= 0) {
        return native_error("Sprite must have frame_width and frame_height set");
    }

    // Must have an image
    if (!sprite->image) {
        return native_error("Sprite must have an image set");
    }

    // Calculate frames per row
    int frames_per_row = sprite->image->width / sprite->frame_width;
    if (frames_per_row <= 0) frames_per_row = 1;

    // Calculate frame position
    int frame_row = frame_index / frames_per_row;
    int frame_col = frame_index % frames_per_row;

    sprite->frame_x = frame_col * sprite->frame_width;
    sprite->frame_y = frame_row * sprite->frame_height;

    return NONE_VAL;
}
// LCOV_EXCL_STOP

// ============================================================================
// Font and Text Functions
// ============================================================================

// LCOV_EXCL_START - font functions require PAL font loading
// load_font(path, size) -> font
static Value native_load_font(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_STRING(args[0])) {
        return native_error("load_font() requires a string path as first argument");
    }
    if (!IS_NUMBER(args[1])) {
        return native_error("load_font() requires a number size as second argument");
    }

    const char* path = AS_CSTRING(args[0]);
    int size = (int)AS_NUMBER(args[1]);

    PalFont* pal_font = pal_font_load(path, size);
    if (!pal_font) {
        return native_error("Failed to load font");
    }

    ObjFont* font = font_new(pal_font, size, false);
    return OBJECT_VAL(font);
}

// default_font(size) -> font
static Value native_default_font(int arg_count, Value* args) {
    (void)arg_count;

    int size = 16;  // Default size
    if (arg_count >= 1 && IS_NUMBER(args[0])) {
        size = (int)AS_NUMBER(args[0]);
    }

    PalFont* pal_font = pal_font_default(size);
    if (!pal_font) {
        return native_error("Failed to create default font");
    }

    ObjFont* font = font_new(pal_font, size, true);
    return OBJECT_VAL(font);
}

// draw_text(text, x, y, font, color) -> nil
static Value native_draw_text(int arg_count, Value* args) {
    (void)arg_count;

    Engine* engine = engine_get();
    if (!engine || !engine->window) {
        return NONE_VAL;
    }

    if (!IS_STRING(args[0])) {
        return native_error("draw_text() requires text as first argument");
    }
    if (!IS_NUMBER(args[1]) || !IS_NUMBER(args[2])) {
        return native_error("draw_text() requires x and y as numbers");
    }
    if (!IS_FONT(args[3])) {
        return native_error("draw_text() requires a font as fourth argument");
    }
    if (!IS_NUMBER(args[4])) {
        return native_error("draw_text() requires a color as fifth argument");
    }

    const char* text = AS_CSTRING(args[0]);
    int x = (int)AS_NUMBER(args[1]);
    int y = (int)AS_NUMBER(args[2]);
    ObjFont* font = AS_FONT(args[3]);
    uint32_t color = (uint32_t)AS_NUMBER(args[4]);

    uint8_t r, g, b, a;
    unpack_color(color, &r, &g, &b, &a);

    if (font->font) {
        pal_draw_text(engine->window, font->font, text, x, y, r, g, b, a);
    }

    return NONE_VAL;
}

// text_width(text, font) -> number
static Value native_text_width(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_STRING(args[0])) {
        return native_error("text_width() requires text as first argument");
    }
    if (!IS_FONT(args[1])) {
        return native_error("text_width() requires a font as second argument");
    }

    const char* text = AS_CSTRING(args[0]);
    ObjFont* font = AS_FONT(args[1]);

    int width = 0, height = 0;
    if (font->font) {
        pal_text_size(font->font, text, &width, &height);
    }

    return NUMBER_VAL((double)width);
}

// text_height(text, font) -> number
static Value native_text_height(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_STRING(args[0])) {
        return native_error("text_height() requires text as first argument");
    }
    if (!IS_FONT(args[1])) {
        return native_error("text_height() requires a font as second argument");
    }

    const char* text = AS_CSTRING(args[0]);
    ObjFont* font = AS_FONT(args[1]);

    int width = 0, height = 0;
    if (font->font) {
        pal_text_size(font->font, text, &width, &height);
    }

    return NUMBER_VAL((double)height);
}
// LCOV_EXCL_STOP

// ============================================================================
// Audio Functions
// ============================================================================

// LCOV_EXCL_START - audio functions require PAL audio loading
// load_sound(path) -> sound
static Value native_load_sound(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_STRING(args[0])) {
        return native_error("load_sound() requires a string path");
    }

    const char* path = AS_CSTRING(args[0]);
    PalSound* pal_sound = pal_sound_load(path);
    if (!pal_sound) {
        return native_error("Failed to load sound");
    }

    ObjString* path_str = string_copy(path, (int)strlen(path));
    ObjSound* sound = sound_new(pal_sound, path_str);
    return OBJECT_VAL(sound);
}

// play_sound(sound) -> nil
static Value native_play_sound(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_SOUND(args[0])) {
        return native_error("play_sound() requires a sound");
    }

    ObjSound* sound = AS_SOUND(args[0]);
    if (sound->sound) {
        pal_sound_play(sound->sound);
    }

    return NONE_VAL;
}

// play_sound_volume(sound, volume) -> nil
static Value native_play_sound_volume(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_SOUND(args[0])) {
        return native_error("play_sound_volume() requires a sound as first argument");
    }
    if (!IS_NUMBER(args[1])) {
        return native_error("play_sound_volume() requires a volume as second argument");
    }

    ObjSound* sound = AS_SOUND(args[0]);
    float volume = (float)AS_NUMBER(args[1]);

    // Clamp volume to 0.0-1.0
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;

    if (sound->sound) {
        pal_sound_play_volume(sound->sound, volume);
    }

    return NONE_VAL;
}

// load_music(path) -> music
static Value native_load_music(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_STRING(args[0])) {
        return native_error("load_music() requires a string path");
    }

    const char* path = AS_CSTRING(args[0]);
    PalMusic* pal_music = pal_music_load(path);
    if (!pal_music) {
        return native_error("Failed to load music");
    }

    ObjString* path_str = string_copy(path, (int)strlen(path));
    ObjMusic* music = music_new(pal_music, path_str);
    return OBJECT_VAL(music);
}

// play_music(music) -> nil
static Value native_play_music(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_MUSIC(args[0])) {
        return native_error("play_music() requires a music object");
    }

    ObjMusic* music = AS_MUSIC(args[0]);
    if (music->music) {
        pal_music_play(music->music, false);  // Play once
    }

    return NONE_VAL;
}

// play_music_loop(music) -> nil
static Value native_play_music_loop(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_MUSIC(args[0])) {
        return native_error("play_music_loop() requires a music object");
    }

    ObjMusic* music = AS_MUSIC(args[0]);
    if (music->music) {
        pal_music_play(music->music, true);  // Play looping
    }

    return NONE_VAL;
}

// pause_music() -> nil
static Value native_pause_music(int arg_count, Value* args) {
    (void)arg_count;
    (void)args;

    pal_music_pause();
    return NONE_VAL;
}

// resume_music() -> nil
static Value native_resume_music(int arg_count, Value* args) {
    (void)arg_count;
    (void)args;

    pal_music_resume();
    return NONE_VAL;
}

// stop_music() -> nil
static Value native_stop_music(int arg_count, Value* args) {
    (void)arg_count;
    (void)args;

    pal_music_stop();
    return NONE_VAL;
}

// set_music_volume(volume) -> nil
static Value native_set_music_volume(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_NUMBER(args[0])) {
        return native_error("set_music_volume() requires a number");
    }

    float volume = (float)AS_NUMBER(args[0]);

    // Clamp volume to 0.0-1.0
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;

    pal_music_set_volume(volume);
    return NONE_VAL;
}

// set_master_volume(volume) -> nil
static Value native_set_master_volume(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_NUMBER(args[0])) {
        return native_error("set_master_volume() requires a number");
    }

    float volume = (float)AS_NUMBER(args[0]);

    // Clamp volume to 0.0-1.0
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;

    pal_set_master_volume(volume);
    return NONE_VAL;
}

// music_playing() -> bool
static Value native_music_playing(int arg_count, Value* args) {
    (void)arg_count;
    (void)args;

    return BOOL_VAL(pal_music_is_playing());
}
// LCOV_EXCL_STOP

// ============================================================================
// Timing Functions
// ============================================================================

// delta_time() -> number
static Value native_delta_time(int arg_count, Value* args) {
    (void)arg_count;
    (void)args;

    Engine* engine = engine_get();
    if (!engine) {
        return NUMBER_VAL(0);  // LCOV_EXCL_LINE
    }

    return NUMBER_VAL(engine->delta_time);
}

// game_time() -> number
static Value native_game_time(int arg_count, Value* args) {
    (void)arg_count;
    (void)args;

    Engine* engine = engine_get();
    if (!engine) {
        return NUMBER_VAL(0);  // LCOV_EXCL_LINE
    }

    return NUMBER_VAL(engine->time);
}

// ============================================================================
// Physics & Collision Functions
// ============================================================================

// set_gravity(strength) -> nil
static Value native_set_gravity(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_NUMBER(args[0])) {
        return native_error("set_gravity() requires a number");  // LCOV_EXCL_LINE
    }

    double gravity = AS_NUMBER(args[0]);
    physics_set_gravity(gravity);
    return NONE_VAL;
}

// get_gravity() -> number
static Value native_get_gravity(int arg_count, Value* args) {
    (void)arg_count;
    (void)args;

    return NUMBER_VAL(physics_get_gravity());
}

// collides(sprite1, sprite2) -> bool
static Value native_collides(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_SPRITE(args[0]) || !IS_SPRITE(args[1])) {
        return native_error("collides() requires two sprites");  // LCOV_EXCL_LINE
    }

    ObjSprite* a = AS_SPRITE(args[0]);
    ObjSprite* b = AS_SPRITE(args[1]);
    return BOOL_VAL(physics_collides(a, b));
}

// collides_rect(sprite, x, y, w, h) -> bool
static Value native_collides_rect(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_SPRITE(args[0])) {
        return native_error("collides_rect() requires a sprite as first argument");  // LCOV_EXCL_LINE
    }
    if (!IS_NUMBER(args[1]) || !IS_NUMBER(args[2]) ||
        !IS_NUMBER(args[3]) || !IS_NUMBER(args[4])) {
        return native_error("collides_rect() requires x, y, w, h as numbers");  // LCOV_EXCL_LINE
    }

    ObjSprite* sprite = AS_SPRITE(args[0]);
    double x = AS_NUMBER(args[1]);
    double y = AS_NUMBER(args[2]);
    double w = AS_NUMBER(args[3]);
    double h = AS_NUMBER(args[4]);
    return BOOL_VAL(physics_collides_rect(sprite, x, y, w, h));
}

// collides_point(sprite, x, y) -> bool
static Value native_collides_point(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_SPRITE(args[0])) {
        return native_error("collides_point() requires a sprite as first argument");  // LCOV_EXCL_LINE
    }
    if (!IS_NUMBER(args[1]) || !IS_NUMBER(args[2])) {
        return native_error("collides_point() requires x, y as numbers");  // LCOV_EXCL_LINE
    }

    ObjSprite* sprite = AS_SPRITE(args[0]);
    double x = AS_NUMBER(args[1]);
    double y = AS_NUMBER(args[2]);
    return BOOL_VAL(physics_collides_point(sprite, x, y));
}

// collides_circle(sprite1, sprite2) -> bool
static Value native_collides_circle(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_SPRITE(args[0]) || !IS_SPRITE(args[1])) {
        return native_error("collides_circle() requires two sprites");  // LCOV_EXCL_LINE
    }

    ObjSprite* a = AS_SPRITE(args[0]);
    ObjSprite* b = AS_SPRITE(args[1]);
    return BOOL_VAL(physics_collides_circle(a, b));
}

// distance(sprite1, sprite2) -> number
static Value native_distance(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_SPRITE(args[0]) || !IS_SPRITE(args[1])) {
        return native_error("distance() requires two sprites");  // LCOV_EXCL_LINE
    }

    ObjSprite* a = AS_SPRITE(args[0]);
    ObjSprite* b = AS_SPRITE(args[1]);
    return NUMBER_VAL(physics_distance(a, b));
}

// apply_force(sprite, fx, fy) -> nil
static Value native_apply_force(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_SPRITE(args[0])) {
        return native_error("apply_force() requires a sprite as first argument");  // LCOV_EXCL_LINE
    }
    if (!IS_NUMBER(args[1]) || !IS_NUMBER(args[2])) {
        return native_error("apply_force() requires fx, fy as numbers");  // LCOV_EXCL_LINE
    }

    ObjSprite* sprite = AS_SPRITE(args[0]);
    double fx = AS_NUMBER(args[1]);
    double fy = AS_NUMBER(args[2]);
    physics_apply_force(sprite, fx, fy);
    return NONE_VAL;
}

// move_toward(sprite, x, y, speed) -> bool
static Value native_move_toward(int arg_count, Value* args) {
    (void)arg_count;

    Engine* engine = engine_get();
    if (!engine) {
        return native_error("No engine initialized");  // LCOV_EXCL_LINE
    }

    if (!IS_SPRITE(args[0])) {
        return native_error("move_toward() requires a sprite as first argument");  // LCOV_EXCL_LINE
    }
    if (!IS_NUMBER(args[1]) || !IS_NUMBER(args[2]) || !IS_NUMBER(args[3])) {
        return native_error("move_toward() requires x, y, speed as numbers");  // LCOV_EXCL_LINE
    }

    ObjSprite* sprite = AS_SPRITE(args[0]);
    double x = AS_NUMBER(args[1]);
    double y = AS_NUMBER(args[2]);
    double speed = AS_NUMBER(args[3]);

    bool reached = physics_move_toward(sprite, x, y, speed, engine->delta_time);
    return BOOL_VAL(reached);
}

// look_at(sprite, x, y) -> nil
static Value native_look_at(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_SPRITE(args[0])) {
        return native_error("look_at() requires a sprite as first argument");  // LCOV_EXCL_LINE
    }
    if (!IS_NUMBER(args[1]) || !IS_NUMBER(args[2])) {
        return native_error("look_at() requires x, y as numbers");  // LCOV_EXCL_LINE
    }

    ObjSprite* sprite = AS_SPRITE(args[0]);
    double x = AS_NUMBER(args[1]);
    double y = AS_NUMBER(args[2]);
    physics_look_at(sprite, x, y);
    return NONE_VAL;
}

// lerp(a, b, t) -> number
static Value native_lerp(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_NUMBER(args[0]) || !IS_NUMBER(args[1]) || !IS_NUMBER(args[2])) {
        return native_error("lerp() requires three numbers");  // LCOV_EXCL_LINE
    }

    double a = AS_NUMBER(args[0]);
    double b = AS_NUMBER(args[1]);
    double t = AS_NUMBER(args[2]);
    return NUMBER_VAL(physics_lerp(a, b, t));
}

// lerp_angle(a, b, t) -> number
static Value native_lerp_angle(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_NUMBER(args[0]) || !IS_NUMBER(args[1]) || !IS_NUMBER(args[2])) {
        return native_error("lerp_angle() requires three numbers");  // LCOV_EXCL_LINE
    }

    double a = AS_NUMBER(args[0]);
    double b = AS_NUMBER(args[1]);
    double t = AS_NUMBER(args[2]);
    return NUMBER_VAL(physics_lerp_angle(a, b, t));
}

// ============================================================================
// Camera Functions
// ============================================================================
// LCOV_EXCL_START - camera functions require engine/camera setup

// Helper to get or create the engine camera
static ObjCamera* get_or_create_camera(void) {
    Engine* engine = engine_get();
    if (!engine) return NULL;

    if (!engine->camera) {
        engine->camera = camera_new();
    }
    return engine->camera;
}

// camera() -> camera object (creates if not exists)
static Value native_camera(int arg_count, Value* args) {
    (void)arg_count;
    (void)args;

    ObjCamera* camera = get_or_create_camera();
    if (!camera) {
        return native_error("No engine initialized");
    }
    return OBJECT_VAL(camera);
}

// camera_x() -> number
static Value native_camera_x(int arg_count, Value* args) {
    (void)arg_count;
    (void)args;

    ObjCamera* camera = get_or_create_camera();
    if (!camera) return NUMBER_VAL(0);
    return NUMBER_VAL(camera->x + camera->shake_offset_x);
}

// camera_y() -> number
static Value native_camera_y(int arg_count, Value* args) {
    (void)arg_count;
    (void)args;

    ObjCamera* camera = get_or_create_camera();
    if (!camera) return NUMBER_VAL(0);
    return NUMBER_VAL(camera->y + camera->shake_offset_y);
}

// camera_zoom() -> number
static Value native_camera_zoom(int arg_count, Value* args) {
    (void)arg_count;
    (void)args;

    ObjCamera* camera = get_or_create_camera();
    if (!camera) return NUMBER_VAL(1);
    return NUMBER_VAL(camera->zoom);
}

// camera_set_position(x, y) -> nil
static Value native_camera_set_position(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_NUMBER(args[0]) || !IS_NUMBER(args[1])) {
        return native_error("camera_set_position() requires x and y as numbers");
    }

    ObjCamera* camera = get_or_create_camera();
    if (!camera) return NONE_VAL;

    camera->x = AS_NUMBER(args[0]);
    camera->y = AS_NUMBER(args[1]);
    camera->target = NULL;  // Stop following
    return NONE_VAL;
}

// camera_set_zoom(zoom) -> nil
static Value native_camera_set_zoom(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_NUMBER(args[0])) {
        return native_error("camera_set_zoom() requires a number");
    }

    ObjCamera* camera = get_or_create_camera();
    if (!camera) return NONE_VAL;

    double zoom = AS_NUMBER(args[0]);
    if (zoom <= 0) zoom = 0.01;  // Prevent invalid zoom
    camera->zoom = zoom;
    return NONE_VAL;
}

// camera_follow(sprite, lerp?) -> nil
static Value native_camera_follow(int arg_count, Value* args) {
    if (!IS_SPRITE(args[0]) && !IS_NONE(args[0])) {
        return native_error("camera_follow() requires a sprite or none");
    }

    ObjCamera* camera = get_or_create_camera();
    if (!camera) return NONE_VAL;

    if (IS_NONE(args[0])) {
        camera->target = NULL;
    } else {
        camera->target = AS_SPRITE(args[0]);

        // Optional lerp factor
        if (arg_count >= 2 && IS_NUMBER(args[1])) {
            double lerp = AS_NUMBER(args[1]);
            if (lerp < 0) lerp = 0;
            if (lerp > 1) lerp = 1;
            camera->follow_lerp = lerp;
        }
    }
    return NONE_VAL;
}

// camera_shake(intensity, duration) -> nil
static Value native_camera_shake(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_NUMBER(args[0]) || !IS_NUMBER(args[1])) {
        return native_error("camera_shake() requires intensity and duration as numbers");
    }

    ObjCamera* camera = get_or_create_camera();
    if (!camera) return NONE_VAL;

    camera->shake_intensity = AS_NUMBER(args[0]);
    camera->shake_duration = AS_NUMBER(args[1]);
    camera->shake_time = 0;
    return NONE_VAL;
}

// screen_to_world_x(x) -> number
static Value native_screen_to_world_x(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_NUMBER(args[0])) {
        return native_error("screen_to_world_x() requires a number");
    }

    Engine* engine = engine_get();
    if (!engine) return args[0];

    double screen_x = AS_NUMBER(args[0]);
    double world_x = screen_x;

    if (engine->camera) {
        ObjCamera* cam = engine->camera;
        int width = engine_get_width(engine);
        // Convert from screen space to world space
        world_x = (screen_x - width / 2.0) / cam->zoom + cam->x + cam->shake_offset_x;
    }

    return NUMBER_VAL(world_x);
}

// screen_to_world_y(y) -> number
static Value native_screen_to_world_y(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_NUMBER(args[0])) {
        return native_error("screen_to_world_y() requires a number");
    }

    Engine* engine = engine_get();
    if (!engine) return args[0];

    double screen_y = AS_NUMBER(args[0]);
    double world_y = screen_y;

    if (engine->camera) {
        ObjCamera* cam = engine->camera;
        int height = engine_get_height(engine);
        // Convert from screen space to world space
        world_y = (screen_y - height / 2.0) / cam->zoom + cam->y + cam->shake_offset_y;
    }

    return NUMBER_VAL(world_y);
}

// world_to_screen_x(x) -> number
static Value native_world_to_screen_x(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_NUMBER(args[0])) {
        return native_error("world_to_screen_x() requires a number");
    }

    Engine* engine = engine_get();
    if (!engine) return args[0];

    double world_x = AS_NUMBER(args[0]);
    double screen_x = world_x;

    if (engine->camera) {
        ObjCamera* cam = engine->camera;
        int width = engine_get_width(engine);
        // Convert from world space to screen space
        screen_x = (world_x - cam->x - cam->shake_offset_x) * cam->zoom + width / 2.0;
    }

    return NUMBER_VAL(screen_x);
}

// world_to_screen_y(y) -> number
static Value native_world_to_screen_y(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_NUMBER(args[0])) {
        return native_error("world_to_screen_y() requires a number");
    }

    Engine* engine = engine_get();
    if (!engine) return args[0];

    double world_y = AS_NUMBER(args[0]);
    double screen_y = world_y;

    if (engine->camera) {
        ObjCamera* cam = engine->camera;
        int height = engine_get_height(engine);
        // Convert from world space to screen space
        screen_y = (world_y - cam->y - cam->shake_offset_y) * cam->zoom + height / 2.0;
    }

    return NUMBER_VAL(screen_y);
}
// LCOV_EXCL_STOP

// ============================================================================
// Animation Functions
// ============================================================================
// LCOV_EXCL_START - animation functions require full setup

// create_animation(image, frame_width, frame_height, frames, frame_time) -> animation
static Value native_create_animation(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_IMAGE(args[0])) {
        return native_error("create_animation() requires an image as first argument");
    }
    if (!IS_NUMBER(args[1]) || !IS_NUMBER(args[2])) {
        return native_error("create_animation() requires frame_width and frame_height as numbers");
    }
    if (!IS_LIST(args[3])) {
        return native_error("create_animation() requires a list of frame indices");
    }
    if (!IS_NUMBER(args[4])) {
        return native_error("create_animation() requires frame_time as a number");
    }

    ObjImage* image = AS_IMAGE(args[0]);
    int frame_width = (int)AS_NUMBER(args[1]);
    int frame_height = (int)AS_NUMBER(args[2]);
    ObjList* frame_list = AS_LIST(args[3]);
    double frame_time = AS_NUMBER(args[4]);

    ObjAnimation* anim = animation_new(image, frame_width, frame_height);

    // Convert list to frame array
    if (frame_list->count > 0) {
        int* frames = PH_ALLOC(sizeof(int) * frame_list->count);
        for (int i = 0; i < frame_list->count; i++) {
            if (IS_NUMBER(frame_list->items[i])) {
                frames[i] = (int)AS_NUMBER(frame_list->items[i]);
            } else {
                frames[i] = 0;
            }
        }
        animation_set_frames(anim, frames, frame_list->count, frame_time);
        PH_FREE(frames);
    }

    return OBJECT_VAL(anim);
}

// animation_play(animation) -> nil
static Value native_animation_play(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_ANIMATION(args[0])) {
        return native_error("animation_play() requires an animation");
    }

    ObjAnimation* anim = AS_ANIMATION(args[0]);
    anim->playing = true;
    return NONE_VAL;
}

// animation_stop(animation) -> nil
static Value native_animation_stop(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_ANIMATION(args[0])) {
        return native_error("animation_stop() requires an animation");
    }

    ObjAnimation* anim = AS_ANIMATION(args[0]);
    anim->playing = false;
    return NONE_VAL;
}

// animation_reset(animation) -> nil
static Value native_animation_reset(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_ANIMATION(args[0])) {
        return native_error("animation_reset() requires an animation");
    }

    ObjAnimation* anim = AS_ANIMATION(args[0]);
    anim->current_frame = 0;
    anim->current_time = 0;
    return NONE_VAL;
}

// animation_set_looping(animation, loop) -> nil
static Value native_animation_set_looping(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_ANIMATION(args[0])) {
        return native_error("animation_set_looping() requires an animation");
    }
    if (!IS_BOOL(args[1])) {
        return native_error("animation_set_looping() requires a boolean");
    }

    ObjAnimation* anim = AS_ANIMATION(args[0]);
    anim->looping = AS_BOOL(args[1]);
    return NONE_VAL;
}

// sprite_set_animation(sprite, animation) -> nil
static Value native_sprite_set_animation(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_SPRITE(args[0])) {
        return native_error("sprite_set_animation() requires a sprite");
    }
    if (!IS_ANIMATION(args[1]) && !IS_NONE(args[1])) {
        return native_error("sprite_set_animation() requires an animation or none");
    }

    ObjSprite* sprite = AS_SPRITE(args[0]);

    if (IS_NONE(args[1])) {
        sprite->animation = NULL;
    } else {
        ObjAnimation* anim = AS_ANIMATION(args[1]);
        sprite->animation = anim;
        // Copy animation settings to sprite
        sprite->frame_width = anim->frame_width;
        sprite->frame_height = anim->frame_height;
        // Use the animation's image if sprite doesn't have one
        if (!sprite->image && anim->image) {
            sprite->image = anim->image;
        }
    }
    return NONE_VAL;
}

// sprite_play(sprite) -> nil
static Value native_sprite_play(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_SPRITE(args[0])) {
        return native_error("sprite_play() requires a sprite");
    }

    ObjSprite* sprite = AS_SPRITE(args[0]);
    if (sprite->animation) {
        sprite->animation->playing = true;
    }
    return NONE_VAL;
}

// sprite_stop(sprite) -> nil
static Value native_sprite_stop(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_SPRITE(args[0])) {
        return native_error("sprite_stop() requires a sprite");
    }

    ObjSprite* sprite = AS_SPRITE(args[0]);
    if (sprite->animation) {
        sprite->animation->playing = false;
    }
    return NONE_VAL;
}

// animation_frame(animation) -> number
static Value native_animation_frame(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_ANIMATION(args[0])) {
        return native_error("animation_frame() requires an animation");
    }

    ObjAnimation* anim = AS_ANIMATION(args[0]);
    return NUMBER_VAL((double)anim->current_frame);
}

// animation_playing(animation) -> bool
static Value native_animation_playing(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_ANIMATION(args[0])) {
        return native_error("animation_playing() requires an animation");
    }

    ObjAnimation* anim = AS_ANIMATION(args[0]);
    return BOOL_VAL(anim->playing);
}
// LCOV_EXCL_STOP

// ============================================================================
// Scene Functions
// ============================================================================
// LCOV_EXCL_START - scene functions require engine setup

// load_scene(name) -> nil
static Value native_load_scene(int arg_count, Value* args) {
    (void)arg_count;

    Engine* engine = engine_get();
    if (!engine) {
        return native_error("No engine initialized");
    }

    if (!IS_STRING(args[0])) {
        return native_error("load_scene() requires a scene name string");
    }

    const char* name = AS_CSTRING(args[0]);
    engine_load_scene(engine, name);
    return NONE_VAL;
}

// get_scene() -> string
static Value native_get_scene(int arg_count, Value* args) {
    (void)arg_count;
    (void)args;

    Engine* engine = engine_get();
    if (!engine) {
        return native_error("No engine initialized");
    }

    const char* scene = engine_get_scene(engine);
    ObjString* str = string_copy(scene, (int)strlen(scene));
    return OBJECT_VAL(str);
}
// LCOV_EXCL_STOP

// ============================================================================
// Particle Functions
// ============================================================================
// LCOV_EXCL_START - particle functions require full particle system setup

// create_emitter(x, y) -> emitter
static Value native_create_emitter(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_NUMBER(args[0]) || !IS_NUMBER(args[1])) {
        return native_error("create_emitter() requires x and y as numbers");
    }

    double x = AS_NUMBER(args[0]);
    double y = AS_NUMBER(args[1]);

    ObjParticleEmitter* emitter = particle_emitter_new(x, y);
    return OBJECT_VAL(emitter);
}

// emitter_emit(emitter, count) -> nil
static Value native_emitter_emit(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_PARTICLE_EMITTER(args[0])) {
        return native_error("emitter_emit() requires a particle emitter");
    }
    if (!IS_NUMBER(args[1])) {
        return native_error("emitter_emit() requires a count as number");
    }

    ObjParticleEmitter* emitter = AS_PARTICLE_EMITTER(args[0]);
    int count = (int)AS_NUMBER(args[1]);
    particle_emitter_emit(emitter, count);
    return NONE_VAL;
}

// emitter_set_color(emitter, color) -> nil
static Value native_emitter_set_color(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_PARTICLE_EMITTER(args[0])) {
        return native_error("emitter_set_color() requires a particle emitter");
    }
    if (!IS_NUMBER(args[1])) {
        return native_error("emitter_set_color() requires a color");
    }

    ObjParticleEmitter* emitter = AS_PARTICLE_EMITTER(args[0]);
    emitter->color = (uint32_t)AS_NUMBER(args[1]);
    return NONE_VAL;
}

// emitter_set_speed(emitter, min, max) -> nil
static Value native_emitter_set_speed(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_PARTICLE_EMITTER(args[0])) {
        return native_error("emitter_set_speed() requires a particle emitter");
    }
    if (!IS_NUMBER(args[1]) || !IS_NUMBER(args[2])) {
        return native_error("emitter_set_speed() requires min and max as numbers");
    }

    ObjParticleEmitter* emitter = AS_PARTICLE_EMITTER(args[0]);
    emitter->speed_min = AS_NUMBER(args[1]);
    emitter->speed_max = AS_NUMBER(args[2]);
    return NONE_VAL;
}

// emitter_set_angle(emitter, min, max) -> nil
static Value native_emitter_set_angle(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_PARTICLE_EMITTER(args[0])) {
        return native_error("emitter_set_angle() requires a particle emitter");
    }
    if (!IS_NUMBER(args[1]) || !IS_NUMBER(args[2])) {
        return native_error("emitter_set_angle() requires min and max as numbers");
    }

    ObjParticleEmitter* emitter = AS_PARTICLE_EMITTER(args[0]);
    emitter->angle_min = AS_NUMBER(args[1]);
    emitter->angle_max = AS_NUMBER(args[2]);
    return NONE_VAL;
}

// emitter_set_lifetime(emitter, min, max) -> nil
static Value native_emitter_set_lifetime(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_PARTICLE_EMITTER(args[0])) {
        return native_error("emitter_set_lifetime() requires a particle emitter");
    }
    if (!IS_NUMBER(args[1]) || !IS_NUMBER(args[2])) {
        return native_error("emitter_set_lifetime() requires min and max as numbers");
    }

    ObjParticleEmitter* emitter = AS_PARTICLE_EMITTER(args[0]);
    emitter->life_min = AS_NUMBER(args[1]);
    emitter->life_max = AS_NUMBER(args[2]);
    return NONE_VAL;
}

// emitter_set_size(emitter, min, max) -> nil
static Value native_emitter_set_size(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_PARTICLE_EMITTER(args[0])) {
        return native_error("emitter_set_size() requires a particle emitter");
    }
    if (!IS_NUMBER(args[1]) || !IS_NUMBER(args[2])) {
        return native_error("emitter_set_size() requires min and max as numbers");
    }

    ObjParticleEmitter* emitter = AS_PARTICLE_EMITTER(args[0]);
    emitter->size_min = AS_NUMBER(args[1]);
    emitter->size_max = AS_NUMBER(args[2]);
    return NONE_VAL;
}

// emitter_set_gravity(emitter, gravity) -> nil
static Value native_emitter_set_gravity(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_PARTICLE_EMITTER(args[0])) {
        return native_error("emitter_set_gravity() requires a particle emitter");
    }
    if (!IS_NUMBER(args[1])) {
        return native_error("emitter_set_gravity() requires gravity as number");
    }

    ObjParticleEmitter* emitter = AS_PARTICLE_EMITTER(args[0]);
    emitter->gravity = AS_NUMBER(args[1]);
    return NONE_VAL;
}

// emitter_set_rate(emitter, rate) -> nil
static Value native_emitter_set_rate(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_PARTICLE_EMITTER(args[0])) {
        return native_error("emitter_set_rate() requires a particle emitter");
    }
    if (!IS_NUMBER(args[1])) {
        return native_error("emitter_set_rate() requires rate as number");
    }

    ObjParticleEmitter* emitter = AS_PARTICLE_EMITTER(args[0]);
    emitter->rate = AS_NUMBER(args[1]);
    return NONE_VAL;
}

// emitter_set_position(emitter, x, y) -> nil
static Value native_emitter_set_position(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_PARTICLE_EMITTER(args[0])) {
        return native_error("emitter_set_position() requires a particle emitter");
    }
    if (!IS_NUMBER(args[1]) || !IS_NUMBER(args[2])) {
        return native_error("emitter_set_position() requires x and y as numbers");
    }

    ObjParticleEmitter* emitter = AS_PARTICLE_EMITTER(args[0]);
    emitter->x = AS_NUMBER(args[1]);
    emitter->y = AS_NUMBER(args[2]);
    return NONE_VAL;
}

// emitter_set_active(emitter, active) -> nil
static Value native_emitter_set_active(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_PARTICLE_EMITTER(args[0])) {
        return native_error("emitter_set_active() requires a particle emitter");
    }
    if (!IS_BOOL(args[1])) {
        return native_error("emitter_set_active() requires a boolean");
    }

    ObjParticleEmitter* emitter = AS_PARTICLE_EMITTER(args[0]);
    emitter->active = AS_BOOL(args[1]);
    return NONE_VAL;
}

// draw_particles(emitter) -> nil
static Value native_draw_particles(int arg_count, Value* args) {
    (void)arg_count;

    Engine* engine = engine_get();
    if (!engine || !engine->window) {
        return NONE_VAL;
    }

    if (!IS_PARTICLE_EMITTER(args[0])) {
        return native_error("draw_particles() requires a particle emitter");
    }

    ObjParticleEmitter* emitter = AS_PARTICLE_EMITTER(args[0]);

    for (int i = 0; i < emitter->particle_count; i++) {
        Particle* p = &emitter->particles[i];

        // Apply camera transform
        int screen_x, screen_y;
        apply_camera_transform(p->x, p->y, &screen_x, &screen_y);
        int size = apply_camera_zoom((int)p->size);
        if (size < 1) size = 1;

        // Unpack color
        uint8_t a = (p->color >> 24) & 0xFF;
        uint8_t r = (p->color >> 16) & 0xFF;
        uint8_t g = (p->color >> 8) & 0xFF;
        uint8_t b = p->color & 0xFF;

        // Draw as filled circle (or rectangle for simplicity)
        pal_draw_rect(engine->window, screen_x - size/2, screen_y - size/2, size, size, r, g, b, a);
    }

    return NONE_VAL;
}

// emitter_count(emitter) -> number
static Value native_emitter_count(int arg_count, Value* args) {
    (void)arg_count;

    if (!IS_PARTICLE_EMITTER(args[0])) {
        return native_error("emitter_count() requires a particle emitter");
    }

    ObjParticleEmitter* emitter = AS_PARTICLE_EMITTER(args[0]);
    return NUMBER_VAL((double)emitter->particle_count);
}
// LCOV_EXCL_STOP

// ============================================================================
// Registration
// ============================================================================

void engine_natives_init(VM* vm) {
    // Color functions
    define_native(vm, "rgb", native_rgb, 3);
    define_native(vm, "rgba", native_rgba, 4);

    // Window functions
    define_native(vm, "create_window", native_create_window, -1);  // variadic
    define_native(vm, "set_title", native_set_title, 1);
    define_native(vm, "window_width", native_window_width, 0);
    define_native(vm, "window_height", native_window_height, 0);

    // Drawing functions
    define_native(vm, "clear", native_clear, 1);
    define_native(vm, "draw_rect", native_draw_rect, 5);
    define_native(vm, "draw_circle", native_draw_circle, 4);
    define_native(vm, "draw_line", native_draw_line, 5);

    // Input functions
    define_native(vm, "key_down", native_key_down, 1);
    define_native(vm, "key_pressed", native_key_pressed, 1);
    define_native(vm, "key_released", native_key_released, 1);
    define_native(vm, "mouse_x", native_mouse_x, 0);
    define_native(vm, "mouse_y", native_mouse_y, 0);
    define_native(vm, "mouse_down", native_mouse_down, 1);
    define_native(vm, "mouse_pressed", native_mouse_pressed, 1);
    define_native(vm, "mouse_released", native_mouse_released, 1);

    // Timing functions
    define_native(vm, "delta_time", native_delta_time, 0);
    define_native(vm, "game_time", native_game_time, 0);

    // Image and sprite functions
    define_native(vm, "load_image", native_load_image, 1);
    define_native(vm, "image_width", native_image_width, 1);
    define_native(vm, "image_height", native_image_height, 1);
    define_native(vm, "draw_image", native_draw_image, 3);
    define_native(vm, "draw_image_ex", native_draw_image_ex, 8);
    define_native(vm, "create_sprite", native_create_sprite, -1);  // variadic (0-1 args)
    define_native(vm, "draw_sprite", native_draw_sprite, 1);
    define_native(vm, "set_sprite_frame", native_set_sprite_frame, 2);

    // Font and text functions
    define_native(vm, "load_font", native_load_font, 2);
    define_native(vm, "default_font", native_default_font, -1);  // variadic (0-1 args)
    define_native(vm, "draw_text", native_draw_text, 5);
    define_native(vm, "text_width", native_text_width, 2);
    define_native(vm, "text_height", native_text_height, 2);

    // Audio functions
    define_native(vm, "load_sound", native_load_sound, 1);
    define_native(vm, "play_sound", native_play_sound, 1);
    define_native(vm, "play_sound_volume", native_play_sound_volume, 2);
    define_native(vm, "load_music", native_load_music, 1);
    define_native(vm, "play_music", native_play_music, 1);
    define_native(vm, "play_music_loop", native_play_music_loop, 1);
    define_native(vm, "pause_music", native_pause_music, 0);
    define_native(vm, "resume_music", native_resume_music, 0);
    define_native(vm, "stop_music", native_stop_music, 0);
    define_native(vm, "set_music_volume", native_set_music_volume, 1);
    define_native(vm, "set_master_volume", native_set_master_volume, 1);
    define_native(vm, "music_playing", native_music_playing, 0);

    // Physics and collision functions
    define_native(vm, "set_gravity", native_set_gravity, 1);
    define_native(vm, "get_gravity", native_get_gravity, 0);
    define_native(vm, "collides", native_collides, 2);
    define_native(vm, "collides_rect", native_collides_rect, 5);
    define_native(vm, "collides_point", native_collides_point, 3);
    define_native(vm, "collides_circle", native_collides_circle, 2);
    define_native(vm, "distance", native_distance, 2);
    define_native(vm, "apply_force", native_apply_force, 3);
    define_native(vm, "move_toward", native_move_toward, 4);
    define_native(vm, "look_at", native_look_at, 3);
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
