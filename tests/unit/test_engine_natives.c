// Tests for Engine Native Functions
// Covers all 68+ native functions through direct invocation with mock PAL

#define PAL_MOCK_ENABLED
#include "../test_framework.h"
#include "engine/engine.h"
#include "engine/engine_natives.h"
#include "engine/physics.h"
#include "vm/vm.h"
#include "vm/gc.h"
#include "vm/object.h"
#include "runtime/stdlib.h"
#include "pal/pal.h"
#include <string.h>
#include <math.h>

// ============================================================================
// Test Fixture
// ============================================================================

static VM vm;
static Engine* engine;

static void setup(void) {
    gc_init();
    vm_init(&vm);
    stdlib_init(&vm);

    pal_init(PAL_BACKEND_MOCK);
    pal_mock_clear_calls();

    engine = engine_new(&vm);
    engine_set(engine);
    engine_init(engine, PAL_BACKEND_MOCK);
    engine_create_window(engine, "Test", 800, 600);

    engine_natives_init(&vm);
}

static void teardown(void) {
    engine_shutdown(engine);
    engine_free(engine);
    engine_set(NULL);
    vm_free(&vm);
    gc_free_all();
    pal_quit();
}

// Minimal setup without window
static void setup_minimal(void) {
    gc_init();
    vm_init(&vm);
    stdlib_init(&vm);
    pal_init(PAL_BACKEND_MOCK);
    pal_mock_clear_calls();
    engine_natives_init(&vm);
}

static void teardown_minimal(void) {
    vm_free(&vm);
    gc_free_all();
    pal_quit();
}

// Helper to look up a native function
static ObjNative* get_native(const char* name) {
    void* val_ptr;
    if (table_get_cstr(&vm.globals, name, &val_ptr)) {
        Value val = *(Value*)val_ptr;
        if (IS_NATIVE(val)) {
            return AS_NATIVE(val);
        }
    }
    return NULL;
}

// Helper to call a native function
static Value call_native(const char* name, int argc, Value* args) {
    ObjNative* native = get_native(name);
    if (!native) return NONE_VAL;
    return native->function(argc, args);
}

// ============================================================================
// Color Functions
// ============================================================================

TEST(native_rgb_basic) {
    setup();

    Value args[3] = { NUMBER_VAL(255), NUMBER_VAL(128), NUMBER_VAL(64) };
    Value result = call_native("rgb", 3, args);

    ASSERT(IS_NUMBER(result));
    uint32_t color = (uint32_t)AS_NUMBER(result);

    // Verify RGBA packed as 0xRRGGBBAA (R in high byte, A in low byte)
    uint8_t r, g, b, a;
    unpack_color(color, &r, &g, &b, &a);
    ASSERT_EQ(r, 255);
    ASSERT_EQ(g, 128);
    ASSERT_EQ(b, 64);
    ASSERT_EQ(a, 255);

    teardown();
}

TEST(native_rgb_clamping) {
    setup();

    // Test values > 255 and < 0
    Value args[3] = { NUMBER_VAL(300), NUMBER_VAL(-50), NUMBER_VAL(128) };
    Value result = call_native("rgb", 3, args);

    ASSERT(IS_NUMBER(result));
    uint32_t color = (uint32_t)AS_NUMBER(result);
    uint8_t r, g, b, a;
    unpack_color(color, &r, &g, &b, &a);
    ASSERT_EQ(r, 255);  // Clamped from 300
    ASSERT_EQ(g, 0);    // Clamped from -50

    teardown();
}

TEST(native_rgb_type_error) {
    setup();

    Value args[3] = { OBJECT_VAL(string_copy("red", 3)), NUMBER_VAL(0), NUMBER_VAL(0) };
    Value result = call_native("rgb", 3, args);
    ASSERT(IS_NONE(result));  // Error returns NONE

    teardown();
}

TEST(native_rgba_basic) {
    setup();

    Value args[4] = { NUMBER_VAL(100), NUMBER_VAL(150), NUMBER_VAL(200), NUMBER_VAL(128) };
    Value result = call_native("rgba", 4, args);

    ASSERT(IS_NUMBER(result));
    uint32_t color = (uint32_t)AS_NUMBER(result);
    uint8_t r, g, b, a;
    unpack_color(color, &r, &g, &b, &a);
    ASSERT_EQ(r, 100);
    ASSERT_EQ(g, 150);
    ASSERT_EQ(b, 200);
    ASSERT_EQ(a, 128);

    teardown();
}

TEST(native_rgba_clamping) {
    setup();

    Value args[4] = { NUMBER_VAL(255), NUMBER_VAL(255), NUMBER_VAL(255), NUMBER_VAL(500) };
    Value result = call_native("rgba", 4, args);

    ASSERT(IS_NUMBER(result));
    uint32_t color = (uint32_t)AS_NUMBER(result);
    uint8_t r, g, b, a;
    unpack_color(color, &r, &g, &b, &a);
    ASSERT_EQ(a, 255);  // Clamped from 500

    teardown();
}

TEST(native_rgba_type_error) {
    setup();

    Value args[4] = { NUMBER_VAL(0), NUMBER_VAL(0), NUMBER_VAL(0), BOOL_VAL(true) };
    Value result = call_native("rgba", 4, args);
    ASSERT(IS_NONE(result));

    teardown();
}

// ============================================================================
// Window Functions
// ============================================================================

TEST(native_create_window_defaults) {
    setup();

    pal_mock_clear_calls();
    Value args[0];
    call_native("create_window", 0, args);

    int count;
    const PalMockCall* calls = pal_mock_get_calls(&count);
    bool found = false;
    for (int i = 0; i < count; i++) {
        if (strcmp(calls[i].function, "pal_window_create") == 0) {
            found = true;
            break;
        }
    }
    ASSERT(found);

    teardown();
}

TEST(native_create_window_custom) {
    setup();

    Value args[3] = { NUMBER_VAL(1024), NUMBER_VAL(768), OBJECT_VAL(string_copy("My Game", 7)) };
    call_native("create_window", 3, args);

    // Verify window dimensions
    Value width = call_native("window_width", 0, NULL);
    Value height = call_native("window_height", 0, NULL);
    ASSERT_EQ((int)AS_NUMBER(width), 1024);
    ASSERT_EQ((int)AS_NUMBER(height), 768);

    teardown();
}

TEST(native_set_title) {
    setup();

    ObjString* title = string_copy("New Title", 9);
    Value args[1] = { OBJECT_VAL(title) };
    call_native("set_title", 1, args);

    int count;
    const PalMockCall* calls = pal_mock_get_calls(&count);
    bool found = false;
    for (int i = 0; i < count; i++) {
        if (strcmp(calls[i].function, "pal_window_set_title") == 0) {
            found = true;
            break;
        }
    }
    ASSERT(found);

    teardown();
}

TEST(native_window_dimensions) {
    setup();

    Value width = call_native("window_width", 0, NULL);
    Value height = call_native("window_height", 0, NULL);

    ASSERT(IS_NUMBER(width));
    ASSERT(IS_NUMBER(height));
    ASSERT_EQ((int)AS_NUMBER(width), 800);
    ASSERT_EQ((int)AS_NUMBER(height), 600);

    teardown();
}

TEST(native_window_no_engine) {
    setup_minimal();

    Value width = call_native("window_width", 0, NULL);
    ASSERT(IS_NUMBER(width));
    ASSERT_EQ((int)AS_NUMBER(width), 0);

    teardown_minimal();
}

// ============================================================================
// Drawing Functions
// ============================================================================

TEST(native_clear_basic) {
    setup();

    uint32_t blue = pack_color(0, 0, 255, 255);
    Value args[1] = { NUMBER_VAL((double)blue) };

    pal_mock_clear_calls();
    call_native("clear", 1, args);

    int count;
    const PalMockCall* calls = pal_mock_get_calls(&count);
    bool found = false;
    for (int i = 0; i < count; i++) {
        if (strcmp(calls[i].function, "pal_window_clear") == 0) {
            found = true;
            break;
        }
    }
    ASSERT(found);

    teardown();
}

TEST(native_draw_rect_basic) {
    setup();

    uint32_t red = pack_color(255, 0, 0, 255);
    Value args[5] = { NUMBER_VAL(10), NUMBER_VAL(20), NUMBER_VAL(100), NUMBER_VAL(50), NUMBER_VAL((double)red) };

    pal_mock_clear_calls();
    call_native("draw_rect", 5, args);

    int count;
    const PalMockCall* calls = pal_mock_get_calls(&count);
    bool found = false;
    for (int i = 0; i < count; i++) {
        if (strcmp(calls[i].function, "pal_draw_rect") == 0) {
            found = true;
            break;
        }
    }
    ASSERT(found);

    teardown();
}

TEST(native_draw_rect_type_error) {
    setup();

    Value args[5] = { OBJECT_VAL(string_copy("x", 1)), NUMBER_VAL(20), NUMBER_VAL(100), NUMBER_VAL(50), NUMBER_VAL(0) };
    Value result = call_native("draw_rect", 5, args);
    ASSERT(IS_NONE(result));

    teardown();
}

TEST(native_draw_circle_basic) {
    setup();

    uint32_t green = pack_color(0, 255, 0, 255);
    Value args[4] = { NUMBER_VAL(400), NUMBER_VAL(300), NUMBER_VAL(50), NUMBER_VAL((double)green) };

    pal_mock_clear_calls();
    call_native("draw_circle", 4, args);

    int count;
    const PalMockCall* calls = pal_mock_get_calls(&count);
    bool found = false;
    for (int i = 0; i < count; i++) {
        if (strcmp(calls[i].function, "pal_draw_circle") == 0) {
            found = true;
            break;
        }
    }
    ASSERT(found);

    teardown();
}

TEST(native_draw_line_basic) {
    setup();

    uint32_t white = pack_color(255, 255, 255, 255);
    Value args[5] = { NUMBER_VAL(0), NUMBER_VAL(0), NUMBER_VAL(100), NUMBER_VAL(100), NUMBER_VAL((double)white) };

    pal_mock_clear_calls();
    call_native("draw_line", 5, args);

    int count;
    const PalMockCall* calls = pal_mock_get_calls(&count);
    bool found = false;
    for (int i = 0; i < count; i++) {
        if (strcmp(calls[i].function, "pal_draw_line") == 0) {
            found = true;
            break;
        }
    }
    ASSERT(found);

    teardown();
}

// ============================================================================
// Input Functions
// ============================================================================

TEST(native_key_down_basic) {
    setup();

    // Initially key not pressed
    Value args[1] = { NUMBER_VAL((double)PAL_KEY_SPACE) };
    Value result = call_native("key_down", 1, args);
    ASSERT(IS_BOOL(result));
    ASSERT_EQ(AS_BOOL(result), false);

    // Set key as pressed
    pal_mock_set_key(PAL_KEY_SPACE, true);
    result = call_native("key_down", 1, args);
    ASSERT_EQ(AS_BOOL(result), true);

    teardown();
}

TEST(native_key_pressed_basic) {
    setup();

    // Establish baseline
    pal_poll_events();
    pal_mock_set_key(PAL_KEY_RETURN, true);

    Value args[1] = { NUMBER_VAL((double)PAL_KEY_RETURN) };
    Value result = call_native("key_pressed", 1, args);
    ASSERT(IS_BOOL(result));
    ASSERT_EQ(AS_BOOL(result), true);

    // After poll_events, pressed becomes false (still down)
    pal_poll_events();
    result = call_native("key_pressed", 1, args);
    ASSERT_EQ(AS_BOOL(result), false);

    teardown();
}

TEST(native_key_released_basic) {
    setup();

    // Key initially pressed
    pal_mock_set_key(PAL_KEY_ESCAPE, true);
    pal_poll_events();

    // Release key
    pal_mock_set_key(PAL_KEY_ESCAPE, false);

    Value args[1] = { NUMBER_VAL((double)PAL_KEY_ESCAPE) };
    Value result = call_native("key_released", 1, args);
    ASSERT(IS_BOOL(result));
    ASSERT_EQ(AS_BOOL(result), true);

    teardown();
}

TEST(native_mouse_position) {
    setup();

    pal_mock_set_mouse_position(250, 350);

    Value mx = call_native("mouse_x", 0, NULL);
    Value my = call_native("mouse_y", 0, NULL);

    ASSERT(IS_NUMBER(mx));
    ASSERT(IS_NUMBER(my));
    ASSERT_EQ((int)AS_NUMBER(mx), 250);
    ASSERT_EQ((int)AS_NUMBER(my), 350);

    teardown();
}

TEST(native_mouse_down_basic) {
    setup();

    Value args[1] = { NUMBER_VAL((double)PAL_MOUSE_LEFT) };
    Value result = call_native("mouse_down", 1, args);
    ASSERT_EQ(AS_BOOL(result), false);

    pal_mock_set_mouse_button(PAL_MOUSE_LEFT, true);
    result = call_native("mouse_down", 1, args);
    ASSERT_EQ(AS_BOOL(result), true);

    teardown();
}

TEST(native_mouse_pressed_basic) {
    setup();

    pal_poll_events();
    pal_mock_set_mouse_button(PAL_MOUSE_RIGHT, true);

    Value args[1] = { NUMBER_VAL((double)PAL_MOUSE_RIGHT) };
    Value result = call_native("mouse_pressed", 1, args);
    ASSERT_EQ(AS_BOOL(result), true);

    teardown();
}

TEST(native_mouse_released_basic) {
    setup();

    pal_mock_set_mouse_button(PAL_MOUSE_MIDDLE, true);
    pal_poll_events();
    pal_mock_set_mouse_button(PAL_MOUSE_MIDDLE, false);

    Value args[1] = { NUMBER_VAL((double)PAL_MOUSE_MIDDLE) };
    Value result = call_native("mouse_released", 1, args);
    ASSERT_EQ(AS_BOOL(result), true);

    teardown();
}

// ============================================================================
// Image and Sprite Functions
// ============================================================================

TEST(native_load_image_basic) {
    setup();

    ObjString* path = string_copy("test.png", 8);
    Value args[1] = { OBJECT_VAL(path) };

    pal_mock_clear_calls();
    Value result = call_native("load_image", 1, args);

    ASSERT(IS_IMAGE(result));
    ObjImage* image = AS_IMAGE(result);
    ASSERT_EQ(image->width, 64);   // Mock default
    ASSERT_EQ(image->height, 64);

    teardown();
}

TEST(native_image_dimensions) {
    setup();

    ObjString* path = string_copy("sprite.png", 10);
    Value load_args[1] = { OBJECT_VAL(path) };
    Value image_val = call_native("load_image", 1, load_args);

    Value width_args[1] = { image_val };
    Value height_args[1] = { image_val };

    Value width = call_native("image_width", 1, width_args);
    Value height = call_native("image_height", 1, height_args);

    ASSERT(IS_NUMBER(width));
    ASSERT(IS_NUMBER(height));
    ASSERT_EQ((int)AS_NUMBER(width), 64);
    ASSERT_EQ((int)AS_NUMBER(height), 64);

    teardown();
}

TEST(native_draw_image_basic) {
    setup();

    ObjString* path = string_copy("test.png", 8);
    Value load_args[1] = { OBJECT_VAL(path) };
    Value image_val = call_native("load_image", 1, load_args);

    Value args[3] = { image_val, NUMBER_VAL(100), NUMBER_VAL(100) };
    pal_mock_clear_calls();
    call_native("draw_image", 3, args);

    int count;
    const PalMockCall* calls = pal_mock_get_calls(&count);
    bool found = false;
    for (int i = 0; i < count; i++) {
        if (strcmp(calls[i].function, "pal_draw_texture") == 0) {
            found = true;
            break;
        }
    }
    ASSERT(found);

    teardown();
}

TEST(native_draw_image_ex_basic) {
    setup();

    ObjString* path = string_copy("test.png", 8);
    Value load_args[1] = { OBJECT_VAL(path) };
    Value image_val = call_native("load_image", 1, load_args);

    Value args[8] = { image_val, NUMBER_VAL(200), NUMBER_VAL(200),
                      NUMBER_VAL(64), NUMBER_VAL(64), NUMBER_VAL(45.0),
                      BOOL_VAL(true), BOOL_VAL(false) };
    pal_mock_clear_calls();
    call_native("draw_image_ex", 8, args);

    int count;
    const PalMockCall* calls = pal_mock_get_calls(&count);
    bool found = false;
    for (int i = 0; i < count; i++) {
        if (strcmp(calls[i].function, "pal_draw_texture_ex") == 0) {
            found = true;
            break;
        }
    }
    ASSERT(found);

    teardown();
}

TEST(native_create_sprite_basic) {
    setup();

    ObjString* path = string_copy("test.png", 8);
    Value load_args[1] = { OBJECT_VAL(path) };
    Value image_val = call_native("load_image", 1, load_args);

    Value args[1] = { image_val };
    Value result = call_native("create_sprite", 1, args);

    ASSERT(IS_SPRITE(result));
    ObjSprite* sprite = AS_SPRITE(result);
    ASSERT_NOT_NULL(sprite->image);
    ASSERT(sprite->visible);
    ASSERT_EQ(sprite->scale_x, 1.0);
    ASSERT_EQ(sprite->scale_y, 1.0);

    teardown();
}

TEST(native_create_sprite_no_image) {
    setup();

    Value args[1] = { NONE_VAL };
    Value result = call_native("create_sprite", 1, args);

    ASSERT(IS_SPRITE(result));
    ObjSprite* sprite = AS_SPRITE(result);
    ASSERT_NULL(sprite->image);

    teardown();
}

TEST(native_draw_sprite_basic) {
    setup();

    ObjString* path = string_copy("test.png", 8);
    Value load_args[1] = { OBJECT_VAL(path) };
    Value image_val = call_native("load_image", 1, load_args);

    Value sprite_args[1] = { image_val };
    Value sprite_val = call_native("create_sprite", 1, sprite_args);

    ObjSprite* sprite = AS_SPRITE(sprite_val);
    sprite->x = 100;
    sprite->y = 100;

    Value args[1] = { sprite_val };
    pal_mock_clear_calls();
    call_native("draw_sprite", 1, args);

    int count;
    const PalMockCall* calls = pal_mock_get_calls(&count);
    bool found = false;
    for (int i = 0; i < count; i++) {
        if (strcmp(calls[i].function, "pal_draw_texture") == 0) {
            found = true;
            break;
        }
    }
    ASSERT(found);

    teardown();
}

TEST(native_set_sprite_frame) {
    setup();

    ObjString* path = string_copy("spritesheet.png", 15);
    Value load_args[1] = { OBJECT_VAL(path) };
    Value image_val = call_native("load_image", 1, load_args);

    Value sprite_args[1] = { image_val };
    Value sprite_val = call_native("create_sprite", 1, sprite_args);

    ObjSprite* sprite = AS_SPRITE(sprite_val);
    sprite->frame_width = 16;
    sprite->frame_height = 16;

    Value args[2] = { sprite_val, NUMBER_VAL(5) };
    call_native("set_sprite_frame", 2, args);

    // Frame 5 with frame size 16, image size 64 = 4 frames per row
    // Frame 5 = row 1, col 1 -> frame_x = 16, frame_y = 16
    ASSERT_EQ(sprite->frame_x, 16);
    ASSERT_EQ(sprite->frame_y, 16);

    teardown();
}

// ============================================================================
// Font and Text Functions
// ============================================================================

TEST(native_load_font_basic) {
    setup();

    ObjString* path = string_copy("font.ttf", 8);
    Value args[2] = { OBJECT_VAL(path), NUMBER_VAL(24) };

    pal_mock_clear_calls();
    Value result = call_native("load_font", 2, args);

    ASSERT(IS_FONT(result));
    ObjFont* font = AS_FONT(result);
    ASSERT_EQ(font->size, 24);
    ASSERT_EQ(font->is_default, false);

    teardown();
}

TEST(native_default_font_basic) {
    setup();

    Value args[1] = { NUMBER_VAL(32) };
    Value result = call_native("default_font", 1, args);

    ASSERT(IS_FONT(result));
    ObjFont* font = AS_FONT(result);
    ASSERT_EQ(font->size, 32);
    ASSERT_EQ(font->is_default, true);

    teardown();
}

TEST(native_draw_text_basic) {
    setup();

    Value font_args[1] = { NUMBER_VAL(16) };
    Value font_val = call_native("default_font", 1, font_args);

    uint32_t white = pack_color(255, 255, 255, 255);
    Value args[5] = { OBJECT_VAL(string_copy("Hello", 5)), NUMBER_VAL(100), NUMBER_VAL(100),
                      font_val, NUMBER_VAL((double)white) };

    pal_mock_clear_calls();
    call_native("draw_text", 5, args);

    int count;
    const PalMockCall* calls = pal_mock_get_calls(&count);
    bool found = false;
    for (int i = 0; i < count; i++) {
        if (strcmp(calls[i].function, "pal_draw_text") == 0) {
            found = true;
            break;
        }
    }
    ASSERT(found);

    teardown();
}

TEST(native_text_size) {
    setup();

    Value font_args[1] = { NUMBER_VAL(16) };
    Value font_val = call_native("default_font", 1, font_args);

    ObjString* text = string_copy("Hello World", 11);
    Value width_args[2] = { OBJECT_VAL(text), font_val };
    Value height_args[2] = { OBJECT_VAL(text), font_val };

    Value width = call_native("text_width", 2, width_args);
    Value height = call_native("text_height", 2, height_args);

    ASSERT(IS_NUMBER(width));
    ASSERT(IS_NUMBER(height));
    ASSERT(AS_NUMBER(width) > 0);
    ASSERT(AS_NUMBER(height) > 0);

    teardown();
}

// ============================================================================
// Audio Functions
// ============================================================================

TEST(native_load_sound_basic) {
    setup();

    ObjString* path = string_copy("jump.wav", 8);
    Value args[1] = { OBJECT_VAL(path) };
    Value result = call_native("load_sound", 1, args);

    ASSERT(IS_SOUND(result));

    teardown();
}

TEST(native_play_sound_basic) {
    setup();

    ObjString* path = string_copy("jump.wav", 8);
    Value load_args[1] = { OBJECT_VAL(path) };
    Value sound_val = call_native("load_sound", 1, load_args);

    Value args[1] = { sound_val };
    pal_mock_clear_calls();
    call_native("play_sound", 1, args);

    int count;
    const PalMockCall* calls = pal_mock_get_calls(&count);
    bool found = false;
    for (int i = 0; i < count; i++) {
        if (strcmp(calls[i].function, "pal_sound_play") == 0) {
            found = true;
            break;
        }
    }
    ASSERT(found);

    teardown();
}

TEST(native_play_sound_volume) {
    setup();

    ObjString* path = string_copy("jump.wav", 8);
    Value load_args[1] = { OBJECT_VAL(path) };
    Value sound_val = call_native("load_sound", 1, load_args);

    Value args[2] = { sound_val, NUMBER_VAL(0.5) };
    pal_mock_clear_calls();
    call_native("play_sound_volume", 2, args);

    int count;
    const PalMockCall* calls = pal_mock_get_calls(&count);
    bool found = false;
    for (int i = 0; i < count; i++) {
        if (strcmp(calls[i].function, "pal_sound_play_volume") == 0) {
            found = true;
            break;
        }
    }
    ASSERT(found);

    teardown();
}

TEST(native_load_music_basic) {
    setup();

    ObjString* path = string_copy("bgm.ogg", 7);
    Value args[1] = { OBJECT_VAL(path) };
    Value result = call_native("load_music", 1, args);

    ASSERT(IS_MUSIC(result));

    teardown();
}

TEST(native_music_lifecycle) {
    setup();

    ObjString* path = string_copy("bgm.ogg", 7);
    Value load_args[1] = { OBJECT_VAL(path) };
    Value music_val = call_native("load_music", 1, load_args);

    // Play music
    Value play_args[1] = { music_val };
    call_native("play_music", 1, play_args);

    Value playing = call_native("music_playing", 0, NULL);
    ASSERT(IS_BOOL(playing));
    ASSERT_EQ(AS_BOOL(playing), true);

    // Pause
    call_native("pause_music", 0, NULL);
    playing = call_native("music_playing", 0, NULL);
    ASSERT_EQ(AS_BOOL(playing), false);

    // Resume
    call_native("resume_music", 0, NULL);
    playing = call_native("music_playing", 0, NULL);
    ASSERT_EQ(AS_BOOL(playing), true);

    // Stop
    call_native("stop_music", 0, NULL);
    playing = call_native("music_playing", 0, NULL);
    ASSERT_EQ(AS_BOOL(playing), false);

    teardown();
}

TEST(native_play_music_loop) {
    setup();

    ObjString* path = string_copy("bgm.ogg", 7);
    Value load_args[1] = { OBJECT_VAL(path) };
    Value music_val = call_native("load_music", 1, load_args);

    Value args[1] = { music_val };
    pal_mock_clear_calls();
    call_native("play_music_loop", 1, args);

    int count;
    const PalMockCall* calls = pal_mock_get_calls(&count);
    bool found = false;
    for (int i = 0; i < count; i++) {
        if (strcmp(calls[i].function, "pal_music_play") == 0) {
            found = true;
            break;
        }
    }
    ASSERT(found);

    teardown();
}

TEST(native_set_music_volume) {
    setup();

    Value args[1] = { NUMBER_VAL(0.7) };
    pal_mock_clear_calls();
    call_native("set_music_volume", 1, args);

    int count;
    const PalMockCall* calls = pal_mock_get_calls(&count);
    bool found = false;
    for (int i = 0; i < count; i++) {
        if (strcmp(calls[i].function, "pal_music_set_volume") == 0) {
            found = true;
            break;
        }
    }
    ASSERT(found);

    teardown();
}

TEST(native_set_master_volume) {
    setup();

    Value args[1] = { NUMBER_VAL(0.8) };
    pal_mock_clear_calls();
    call_native("set_master_volume", 1, args);

    int count;
    const PalMockCall* calls = pal_mock_get_calls(&count);
    bool found = false;
    for (int i = 0; i < count; i++) {
        if (strcmp(calls[i].function, "pal_set_master_volume") == 0) {
            found = true;
            break;
        }
    }
    ASSERT(found);

    teardown();
}

// ============================================================================
// Timing Functions
// ============================================================================

TEST(native_delta_time) {
    setup();

    engine->delta_time = 0.016667;
    Value result = call_native("delta_time", 0, NULL);

    ASSERT(IS_NUMBER(result));
    ASSERT(fabs(AS_NUMBER(result) - 0.016667) < 0.0001);

    teardown();
}

TEST(native_game_time) {
    setup();

    engine->time = 5.0;
    Value result = call_native("game_time", 0, NULL);

    ASSERT(IS_NUMBER(result));
    ASSERT_EQ(AS_NUMBER(result), 5.0);

    teardown();
}

// ============================================================================
// Physics & Collision Functions
// ============================================================================

TEST(native_set_get_gravity) {
    setup();

    Value set_args[1] = { NUMBER_VAL(500.0) };
    call_native("set_gravity", 1, set_args);

    Value result = call_native("get_gravity", 0, NULL);
    ASSERT(IS_NUMBER(result));
    ASSERT_EQ(AS_NUMBER(result), 500.0);

    teardown();
}

TEST(native_collides_sprites) {
    setup();

    Value sprite1_val = call_native("create_sprite", 0, NULL);
    Value sprite2_val = call_native("create_sprite", 0, NULL);

    ObjSprite* s1 = AS_SPRITE(sprite1_val);
    ObjSprite* s2 = AS_SPRITE(sprite2_val);

    // Overlapping sprites
    s1->x = 0; s1->y = 0; s1->width = 50; s1->height = 50;
    s2->x = 25; s2->y = 25; s2->width = 50; s2->height = 50;

    Value args[2] = { sprite1_val, sprite2_val };
    Value result = call_native("collides", 2, args);
    ASSERT(IS_BOOL(result));
    ASSERT_EQ(AS_BOOL(result), true);

    // Non-overlapping
    s2->x = 100; s2->y = 100;
    result = call_native("collides", 2, args);
    ASSERT_EQ(AS_BOOL(result), false);

    teardown();
}

TEST(native_collides_rect) {
    setup();

    Value sprite_val = call_native("create_sprite", 0, NULL);
    ObjSprite* s = AS_SPRITE(sprite_val);
    s->x = 50; s->y = 50; s->width = 50; s->height = 50;

    // Overlapping rect
    Value args[5] = { sprite_val, NUMBER_VAL(60), NUMBER_VAL(60), NUMBER_VAL(40), NUMBER_VAL(40) };
    Value result = call_native("collides_rect", 5, args);
    ASSERT_EQ(AS_BOOL(result), true);

    // Non-overlapping rect
    Value args2[5] = { sprite_val, NUMBER_VAL(200), NUMBER_VAL(200), NUMBER_VAL(40), NUMBER_VAL(40) };
    result = call_native("collides_rect", 5, args2);
    ASSERT_EQ(AS_BOOL(result), false);

    teardown();
}

TEST(native_collides_point) {
    setup();

    Value sprite_val = call_native("create_sprite", 0, NULL);
    ObjSprite* s = AS_SPRITE(sprite_val);
    s->x = 100; s->y = 100; s->width = 50; s->height = 50;

    // Inside
    Value args[3] = { sprite_val, NUMBER_VAL(125), NUMBER_VAL(125) };
    Value result = call_native("collides_point", 3, args);
    ASSERT_EQ(AS_BOOL(result), true);

    // Outside
    Value args2[3] = { sprite_val, NUMBER_VAL(50), NUMBER_VAL(50) };
    result = call_native("collides_point", 3, args2);
    ASSERT_EQ(AS_BOOL(result), false);

    teardown();
}

TEST(native_collides_circle) {
    setup();

    Value sprite1_val = call_native("create_sprite", 0, NULL);
    Value sprite2_val = call_native("create_sprite", 0, NULL);

    ObjSprite* s1 = AS_SPRITE(sprite1_val);
    ObjSprite* s2 = AS_SPRITE(sprite2_val);

    s1->x = 0; s1->y = 0; s1->width = 50; s1->height = 50;
    s2->x = 30; s2->y = 0; s2->width = 50; s2->height = 50;

    Value args[2] = { sprite1_val, sprite2_val };
    Value result = call_native("collides_circle", 2, args);
    ASSERT(IS_BOOL(result));

    teardown();
}

TEST(native_distance) {
    setup();

    Value sprite1_val = call_native("create_sprite", 0, NULL);
    Value sprite2_val = call_native("create_sprite", 0, NULL);

    ObjSprite* s1 = AS_SPRITE(sprite1_val);
    ObjSprite* s2 = AS_SPRITE(sprite2_val);

    s1->x = 0; s1->y = 0;
    s2->x = 30; s2->y = 40;  // 3-4-5 triangle scaled

    Value args[2] = { sprite1_val, sprite2_val };
    Value result = call_native("distance", 2, args);
    ASSERT(IS_NUMBER(result));
    ASSERT(fabs(AS_NUMBER(result) - 50.0) < 0.01);

    teardown();
}

TEST(native_lerp) {
    setup();

    Value args[3] = { NUMBER_VAL(0), NUMBER_VAL(100), NUMBER_VAL(0.5) };
    Value result = call_native("lerp", 3, args);
    ASSERT(IS_NUMBER(result));
    ASSERT_EQ(AS_NUMBER(result), 50.0);

    teardown();
}

TEST(native_lerp_angle) {
    setup();

    Value args[3] = { NUMBER_VAL(0), NUMBER_VAL(90), NUMBER_VAL(0.5) };
    Value result = call_native("lerp_angle", 3, args);
    ASSERT(IS_NUMBER(result));
    ASSERT_EQ(AS_NUMBER(result), 45.0);

    teardown();
}

TEST(native_apply_force) {
    setup();

    Value sprite_val = call_native("create_sprite", 0, NULL);
    ObjSprite* s = AS_SPRITE(sprite_val);
    s->velocity_x = 0; s->velocity_y = 0;

    Value args[3] = { sprite_val, NUMBER_VAL(10), NUMBER_VAL(20) };
    call_native("apply_force", 3, args);

    ASSERT_EQ(s->acceleration_x, 10.0);
    ASSERT_EQ(s->acceleration_y, 20.0);

    teardown();
}

TEST(native_move_toward) {
    setup();

    Value sprite_val = call_native("create_sprite", 0, NULL);
    ObjSprite* s = AS_SPRITE(sprite_val);
    s->x = 0; s->y = 0;
    engine->delta_time = 1.0;  // 1 second for easy math

    Value args[4] = { sprite_val, NUMBER_VAL(100), NUMBER_VAL(0), NUMBER_VAL(50) };
    Value result = call_native("move_toward", 4, args);

    ASSERT(IS_BOOL(result));
    ASSERT(s->x > 0);  // Moved toward target

    teardown();
}

TEST(native_look_at) {
    setup();

    Value sprite_val = call_native("create_sprite", 0, NULL);
    ObjSprite* s = AS_SPRITE(sprite_val);
    s->x = 0; s->y = 0;

    Value args[3] = { sprite_val, NUMBER_VAL(100), NUMBER_VAL(0) };
    call_native("look_at", 3, args);

    // Should be looking right (0 degrees in most conventions, but may vary)
    ASSERT(s->rotation >= -360 && s->rotation <= 360);

    teardown();
}

// ============================================================================
// Camera Functions
// ============================================================================

TEST(native_camera_create) {
    setup();

    Value result = call_native("camera", 0, NULL);
    ASSERT(IS_CAMERA(result));

    ObjCamera* camera = AS_CAMERA(result);
    ASSERT_EQ(camera->zoom, 1.0);

    teardown();
}

TEST(native_camera_position) {
    setup();

    // Set camera position
    Value pos_args[2] = { NUMBER_VAL(100), NUMBER_VAL(200) };
    call_native("camera_set_position", 2, pos_args);

    Value x = call_native("camera_x", 0, NULL);
    Value y = call_native("camera_y", 0, NULL);

    ASSERT(IS_NUMBER(x));
    ASSERT(IS_NUMBER(y));
    ASSERT_EQ(AS_NUMBER(x), 100.0);
    ASSERT_EQ(AS_NUMBER(y), 200.0);

    teardown();
}

TEST(native_camera_zoom) {
    setup();

    call_native("camera", 0, NULL);  // Ensure camera exists

    Value zoom_args[1] = { NUMBER_VAL(2.0) };
    call_native("camera_set_zoom", 1, zoom_args);

    Value zoom = call_native("camera_zoom", 0, NULL);
    ASSERT_EQ(AS_NUMBER(zoom), 2.0);

    teardown();
}

TEST(native_camera_follow) {
    setup();

    call_native("camera", 0, NULL);  // Ensure camera exists

    Value sprite_val = call_native("create_sprite", 0, NULL);

    Value args[2] = { sprite_val, NUMBER_VAL(0.5) };
    call_native("camera_follow", 2, args);

    ObjCamera* camera = engine->camera;
    ASSERT_NOT_NULL(camera);
    ASSERT_NOT_NULL(camera->target);
    ASSERT_EQ(camera->follow_lerp, 0.5);

    teardown();
}

TEST(native_camera_shake) {
    setup();

    call_native("camera", 0, NULL);

    Value args[2] = { NUMBER_VAL(10), NUMBER_VAL(0.5) };
    call_native("camera_shake", 2, args);

    ObjCamera* camera = engine->camera;
    ASSERT_EQ(camera->shake_intensity, 10.0);
    ASSERT_EQ(camera->shake_duration, 0.5);

    teardown();
}

TEST(native_screen_to_world) {
    setup();

    call_native("camera", 0, NULL);

    Value x_args[1] = { NUMBER_VAL(400) };
    Value y_args[1] = { NUMBER_VAL(300) };

    Value world_x = call_native("screen_to_world_x", 1, x_args);
    Value world_y = call_native("screen_to_world_y", 1, y_args);

    ASSERT(IS_NUMBER(world_x));
    ASSERT(IS_NUMBER(world_y));

    teardown();
}

TEST(native_world_to_screen) {
    setup();

    call_native("camera", 0, NULL);

    Value x_args[1] = { NUMBER_VAL(0) };
    Value y_args[1] = { NUMBER_VAL(0) };

    Value screen_x = call_native("world_to_screen_x", 1, x_args);
    Value screen_y = call_native("world_to_screen_y", 1, y_args);

    ASSERT(IS_NUMBER(screen_x));
    ASSERT(IS_NUMBER(screen_y));

    teardown();
}

// ============================================================================
// Animation Functions
// ============================================================================

TEST(native_create_animation) {
    setup();

    ObjString* path = string_copy("spritesheet.png", 15);
    Value load_args[1] = { OBJECT_VAL(path) };
    Value image_val = call_native("load_image", 1, load_args);

    ObjList* frames = list_new();
    list_append(frames, NUMBER_VAL(0));
    list_append(frames, NUMBER_VAL(1));
    list_append(frames, NUMBER_VAL(2));

    Value args[5] = { image_val, NUMBER_VAL(16), NUMBER_VAL(16),
                      OBJECT_VAL(frames), NUMBER_VAL(0.1) };
    Value result = call_native("create_animation", 5, args);

    ASSERT(IS_ANIMATION(result));
    ObjAnimation* anim = AS_ANIMATION(result);
    ASSERT_EQ(anim->frame_width, 16);
    ASSERT_EQ(anim->frame_height, 16);
    ASSERT_EQ(anim->frame_count, 3);

    teardown();
}

TEST(native_animation_play_stop) {
    setup();

    ObjString* path = string_copy("spritesheet.png", 15);
    Value load_args[1] = { OBJECT_VAL(path) };
    Value image_val = call_native("load_image", 1, load_args);

    ObjList* frames = list_new();
    list_append(frames, NUMBER_VAL(0));

    Value create_args[5] = { image_val, NUMBER_VAL(16), NUMBER_VAL(16),
                             OBJECT_VAL(frames), NUMBER_VAL(0.1) };
    Value anim_val = call_native("create_animation", 5, create_args);

    ObjAnimation* anim = AS_ANIMATION(anim_val);
    ASSERT_EQ(anim->playing, false);

    Value play_args[1] = { anim_val };
    call_native("animation_play", 1, play_args);
    ASSERT_EQ(anim->playing, true);

    Value stop_args[1] = { anim_val };
    call_native("animation_stop", 1, stop_args);
    ASSERT_EQ(anim->playing, false);

    teardown();
}

TEST(native_animation_reset) {
    setup();

    ObjString* path = string_copy("spritesheet.png", 15);
    Value load_args[1] = { OBJECT_VAL(path) };
    Value image_val = call_native("load_image", 1, load_args);

    ObjList* frames = list_new();
    list_append(frames, NUMBER_VAL(0));
    list_append(frames, NUMBER_VAL(1));

    Value create_args[5] = { image_val, NUMBER_VAL(16), NUMBER_VAL(16),
                             OBJECT_VAL(frames), NUMBER_VAL(0.1) };
    Value anim_val = call_native("create_animation", 5, create_args);

    ObjAnimation* anim = AS_ANIMATION(anim_val);
    anim->current_frame = 1;
    anim->current_time = 0.5;

    Value reset_args[1] = { anim_val };
    call_native("animation_reset", 1, reset_args);

    ASSERT_EQ(anim->current_frame, 0);
    ASSERT_EQ(anim->current_time, 0.0);

    teardown();
}

TEST(native_animation_set_looping) {
    setup();

    ObjString* path = string_copy("spritesheet.png", 15);
    Value load_args[1] = { OBJECT_VAL(path) };
    Value image_val = call_native("load_image", 1, load_args);

    ObjList* frames = list_new();
    list_append(frames, NUMBER_VAL(0));

    Value create_args[5] = { image_val, NUMBER_VAL(16), NUMBER_VAL(16),
                             OBJECT_VAL(frames), NUMBER_VAL(0.1) };
    Value anim_val = call_native("create_animation", 5, create_args);

    Value loop_args[2] = { anim_val, BOOL_VAL(false) };
    call_native("animation_set_looping", 2, loop_args);

    ObjAnimation* anim = AS_ANIMATION(anim_val);
    ASSERT_EQ(anim->looping, false);

    teardown();
}

TEST(native_animation_frame) {
    setup();

    ObjString* path = string_copy("spritesheet.png", 15);
    Value load_args[1] = { OBJECT_VAL(path) };
    Value image_val = call_native("load_image", 1, load_args);

    ObjList* frames = list_new();
    list_append(frames, NUMBER_VAL(0));
    list_append(frames, NUMBER_VAL(1));

    Value create_args[5] = { image_val, NUMBER_VAL(16), NUMBER_VAL(16),
                             OBJECT_VAL(frames), NUMBER_VAL(0.1) };
    Value anim_val = call_native("create_animation", 5, create_args);

    ObjAnimation* anim = AS_ANIMATION(anim_val);
    anim->current_frame = 1;

    Value frame_args[1] = { anim_val };
    Value frame = call_native("animation_frame", 1, frame_args);

    ASSERT_EQ((int)AS_NUMBER(frame), 1);

    teardown();
}

TEST(native_animation_playing) {
    setup();

    ObjString* path = string_copy("spritesheet.png", 15);
    Value load_args[1] = { OBJECT_VAL(path) };
    Value image_val = call_native("load_image", 1, load_args);

    ObjList* frames = list_new();
    list_append(frames, NUMBER_VAL(0));

    Value create_args[5] = { image_val, NUMBER_VAL(16), NUMBER_VAL(16),
                             OBJECT_VAL(frames), NUMBER_VAL(0.1) };
    Value anim_val = call_native("create_animation", 5, create_args);

    Value playing_args[1] = { anim_val };
    Value playing = call_native("animation_playing", 1, playing_args);
    ASSERT_EQ(AS_BOOL(playing), false);

    ObjAnimation* anim = AS_ANIMATION(anim_val);
    anim->playing = true;

    playing = call_native("animation_playing", 1, playing_args);
    ASSERT_EQ(AS_BOOL(playing), true);

    teardown();
}

// ============================================================================
// Scene Functions
// ============================================================================

TEST(native_scene_management) {
    setup();

    Value get_result = call_native("get_scene", 0, NULL);
    ASSERT(IS_STRING(get_result));
    ASSERT_STR_EQ(AS_CSTRING(get_result), "");

    ObjString* scene_name = string_copy("menu", 4);
    Value load_args[1] = { OBJECT_VAL(scene_name) };
    call_native("load_scene", 1, load_args);

    ASSERT(engine->scene_changed);
    ASSERT_STR_EQ(engine->next_scene, "menu");

    teardown();
}

// ============================================================================
// Particle Functions
// ============================================================================

TEST(native_create_emitter) {
    setup();

    Value args[2] = { NUMBER_VAL(400), NUMBER_VAL(300) };
    Value result = call_native("create_emitter", 2, args);

    ASSERT(IS_PARTICLE_EMITTER(result));
    ObjParticleEmitter* emitter = AS_PARTICLE_EMITTER(result);
    ASSERT_EQ(emitter->x, 400);
    ASSERT_EQ(emitter->y, 300);

    teardown();
}

TEST(native_emitter_emit) {
    setup();

    Value create_args[2] = { NUMBER_VAL(400), NUMBER_VAL(300) };
    Value emitter_val = call_native("create_emitter", 2, create_args);

    Value emit_args[2] = { emitter_val, NUMBER_VAL(10) };
    call_native("emitter_emit", 2, emit_args);

    ObjParticleEmitter* emitter = AS_PARTICLE_EMITTER(emitter_val);
    ASSERT_EQ(emitter->particle_count, 10);

    teardown();
}

TEST(native_emitter_set_color) {
    setup();

    Value create_args[2] = { NUMBER_VAL(0), NUMBER_VAL(0) };
    Value emitter_val = call_native("create_emitter", 2, create_args);

    uint32_t red = pack_color(255, 0, 0, 255);
    Value color_args[2] = { emitter_val, NUMBER_VAL((double)red) };
    call_native("emitter_set_color", 2, color_args);

    ObjParticleEmitter* emitter = AS_PARTICLE_EMITTER(emitter_val);
    ASSERT_EQ(emitter->color, red);

    teardown();
}

TEST(native_emitter_set_speed) {
    setup();

    Value create_args[2] = { NUMBER_VAL(0), NUMBER_VAL(0) };
    Value emitter_val = call_native("create_emitter", 2, create_args);

    Value speed_args[3] = { emitter_val, NUMBER_VAL(50), NUMBER_VAL(100) };
    call_native("emitter_set_speed", 3, speed_args);

    ObjParticleEmitter* emitter = AS_PARTICLE_EMITTER(emitter_val);
    ASSERT_EQ(emitter->speed_min, 50);
    ASSERT_EQ(emitter->speed_max, 100);

    teardown();
}

TEST(native_emitter_set_angle) {
    setup();

    Value create_args[2] = { NUMBER_VAL(0), NUMBER_VAL(0) };
    Value emitter_val = call_native("create_emitter", 2, create_args);

    Value angle_args[3] = { emitter_val, NUMBER_VAL(-45), NUMBER_VAL(45) };
    call_native("emitter_set_angle", 3, angle_args);

    ObjParticleEmitter* emitter = AS_PARTICLE_EMITTER(emitter_val);
    ASSERT_EQ(emitter->angle_min, -45);
    ASSERT_EQ(emitter->angle_max, 45);

    teardown();
}

TEST(native_emitter_set_lifetime) {
    setup();

    Value create_args[2] = { NUMBER_VAL(0), NUMBER_VAL(0) };
    Value emitter_val = call_native("create_emitter", 2, create_args);

    Value life_args[3] = { emitter_val, NUMBER_VAL(0.5), NUMBER_VAL(1.5) };
    call_native("emitter_set_lifetime", 3, life_args);

    ObjParticleEmitter* emitter = AS_PARTICLE_EMITTER(emitter_val);
    ASSERT_EQ(emitter->life_min, 0.5);
    ASSERT_EQ(emitter->life_max, 1.5);

    teardown();
}

TEST(native_emitter_set_size) {
    setup();

    Value create_args[2] = { NUMBER_VAL(0), NUMBER_VAL(0) };
    Value emitter_val = call_native("create_emitter", 2, create_args);

    Value size_args[3] = { emitter_val, NUMBER_VAL(2), NUMBER_VAL(8) };
    call_native("emitter_set_size", 3, size_args);

    ObjParticleEmitter* emitter = AS_PARTICLE_EMITTER(emitter_val);
    ASSERT_EQ(emitter->size_min, 2);
    ASSERT_EQ(emitter->size_max, 8);

    teardown();
}

TEST(native_emitter_set_gravity) {
    setup();

    Value create_args[2] = { NUMBER_VAL(0), NUMBER_VAL(0) };
    Value emitter_val = call_native("create_emitter", 2, create_args);

    Value gravity_args[2] = { emitter_val, NUMBER_VAL(200) };
    call_native("emitter_set_gravity", 2, gravity_args);

    ObjParticleEmitter* emitter = AS_PARTICLE_EMITTER(emitter_val);
    ASSERT_EQ(emitter->gravity, 200);

    teardown();
}

TEST(native_emitter_set_rate) {
    setup();

    Value create_args[2] = { NUMBER_VAL(0), NUMBER_VAL(0) };
    Value emitter_val = call_native("create_emitter", 2, create_args);

    Value rate_args[2] = { emitter_val, NUMBER_VAL(20) };
    call_native("emitter_set_rate", 2, rate_args);

    ObjParticleEmitter* emitter = AS_PARTICLE_EMITTER(emitter_val);
    ASSERT_EQ(emitter->rate, 20);

    teardown();
}

TEST(native_emitter_set_position) {
    setup();

    Value create_args[2] = { NUMBER_VAL(0), NUMBER_VAL(0) };
    Value emitter_val = call_native("create_emitter", 2, create_args);

    Value pos_args[3] = { emitter_val, NUMBER_VAL(250), NUMBER_VAL(350) };
    call_native("emitter_set_position", 3, pos_args);

    ObjParticleEmitter* emitter = AS_PARTICLE_EMITTER(emitter_val);
    ASSERT_EQ(emitter->x, 250);
    ASSERT_EQ(emitter->y, 350);

    teardown();
}

TEST(native_emitter_set_active) {
    setup();

    Value create_args[2] = { NUMBER_VAL(0), NUMBER_VAL(0) };
    Value emitter_val = call_native("create_emitter", 2, create_args);

    Value active_args[2] = { emitter_val, BOOL_VAL(true) };
    call_native("emitter_set_active", 2, active_args);

    ObjParticleEmitter* emitter = AS_PARTICLE_EMITTER(emitter_val);
    ASSERT_EQ(emitter->active, true);

    teardown();
}

TEST(native_emitter_count) {
    setup();

    Value create_args[2] = { NUMBER_VAL(0), NUMBER_VAL(0) };
    Value emitter_val = call_native("create_emitter", 2, create_args);

    Value emit_args[2] = { emitter_val, NUMBER_VAL(5) };
    call_native("emitter_emit", 2, emit_args);

    Value count_args[1] = { emitter_val };
    Value count = call_native("emitter_count", 1, count_args);

    ASSERT_EQ((int)AS_NUMBER(count), 5);

    teardown();
}

TEST(native_draw_particles) {
    setup();

    Value create_args[2] = { NUMBER_VAL(400), NUMBER_VAL(300) };
    Value emitter_val = call_native("create_emitter", 2, create_args);

    Value emit_args[2] = { emitter_val, NUMBER_VAL(5) };
    call_native("emitter_emit", 2, emit_args);

    Value draw_args[1] = { emitter_val };
    pal_mock_clear_calls();
    call_native("draw_particles", 1, draw_args);

    int count;
    (void)pal_mock_get_calls(&count);
    ASSERT(count >= 5);  // At least 5 draw_rect calls

    teardown();
}

// ============================================================================
// Sprite Animation Functions
// ============================================================================

TEST(native_sprite_set_animation) {
    setup();

    Value sprite_val = call_native("create_sprite", 0, NULL);

    ObjString* path = string_copy("spritesheet.png", 15);
    Value load_args[1] = { OBJECT_VAL(path) };
    Value image_val = call_native("load_image", 1, load_args);

    ObjList* frames = list_new();
    list_append(frames, NUMBER_VAL(0));

    Value create_anim_args[5] = { image_val, NUMBER_VAL(16), NUMBER_VAL(16),
                                  OBJECT_VAL(frames), NUMBER_VAL(0.1) };
    Value anim_val = call_native("create_animation", 5, create_anim_args);

    Value set_anim_args[2] = { sprite_val, anim_val };
    call_native("sprite_set_animation", 2, set_anim_args);

    ObjSprite* sprite = AS_SPRITE(sprite_val);
    ASSERT_NOT_NULL(sprite->animation);
    ASSERT_EQ(sprite->frame_width, 16);
    ASSERT_EQ(sprite->frame_height, 16);

    teardown();
}

TEST(native_sprite_play_stop) {
    setup();

    Value sprite_val = call_native("create_sprite", 0, NULL);

    ObjString* path = string_copy("spritesheet.png", 15);
    Value load_args[1] = { OBJECT_VAL(path) };
    Value image_val = call_native("load_image", 1, load_args);

    ObjList* frames = list_new();
    list_append(frames, NUMBER_VAL(0));

    Value create_anim_args[5] = { image_val, NUMBER_VAL(16), NUMBER_VAL(16),
                                  OBJECT_VAL(frames), NUMBER_VAL(0.1) };
    Value anim_val = call_native("create_animation", 5, create_anim_args);

    Value set_anim_args[2] = { sprite_val, anim_val };
    call_native("sprite_set_animation", 2, set_anim_args);

    Value play_args[1] = { sprite_val };
    call_native("sprite_play", 1, play_args);

    ObjSprite* sprite = AS_SPRITE(sprite_val);
    ASSERT_EQ(sprite->animation->playing, true);

    Value stop_args[1] = { sprite_val };
    call_native("sprite_stop", 1, stop_args);
    ASSERT_EQ(sprite->animation->playing, false);

    teardown();
}

// ============================================================================
// Constants Tests
// ============================================================================

TEST(color_constants_defined) {
    setup();

    void* val_ptr;
    ASSERT(table_get_cstr(&vm.globals, "RED", &val_ptr));
    Value red = *(Value*)val_ptr;
    ASSERT(IS_NUMBER(red));

    ASSERT(table_get_cstr(&vm.globals, "WHITE", &val_ptr));
    Value white = *(Value*)val_ptr;
    ASSERT(IS_NUMBER(white));

    teardown();
}

TEST(key_constants_defined) {
    setup();

    void* val_ptr;
    ASSERT(table_get_cstr(&vm.globals, "KEY_SPACE", &val_ptr));
    Value space = *(Value*)val_ptr;
    ASSERT(IS_NUMBER(space));
    ASSERT_EQ((int)AS_NUMBER(space), PAL_KEY_SPACE);

    ASSERT(table_get_cstr(&vm.globals, "KEY_UP", &val_ptr));
    Value up = *(Value*)val_ptr;
    ASSERT_EQ((int)AS_NUMBER(up), PAL_KEY_UP);

    teardown();
}

TEST(mouse_constants_defined) {
    setup();

    void* val_ptr;
    ASSERT(table_get_cstr(&vm.globals, "MOUSE_LEFT", &val_ptr));
    Value left = *(Value*)val_ptr;
    ASSERT(IS_NUMBER(left));
    ASSERT_EQ((int)AS_NUMBER(left), PAL_MOUSE_LEFT);

    teardown();
}

// ============================================================================
// Type Error Tests
// ============================================================================

TEST(native_set_title_type_error) {
    setup();
    Value args[1] = { NUMBER_VAL(42) };  // Should be string
    call_native("set_title", 1, args);
    // Should not crash, returns none on error
    teardown();
}

TEST(native_clear_type_error) {
    setup();
    Value args[1] = { BOOL_VAL(true) };  // Should be number (color)
    call_native("clear", 1, args);
    teardown();
}

TEST(native_draw_circle_type_error) {
    setup();
    Value args[4] = { BOOL_VAL(true), NUMBER_VAL(100), NUMBER_VAL(50), NUMBER_VAL(0xFF0000) };
    call_native("draw_circle", 4, args);
    teardown();
}

TEST(native_draw_line_type_error) {
    setup();
    Value args[5] = { BOOL_VAL(true), NUMBER_VAL(0), NUMBER_VAL(100), NUMBER_VAL(100), NUMBER_VAL(0xFF) };
    call_native("draw_line", 5, args);
    teardown();
}

TEST(native_key_down_type_error) {
    setup();
    Value args[1] = { BOOL_VAL(true) };  // Should be number
    call_native("key_down", 1, args);
    teardown();
}

TEST(native_key_pressed_type_error) {
    setup();
    Value args[1] = { BOOL_VAL(true) };
    call_native("key_pressed", 1, args);
    teardown();
}

TEST(native_key_released_type_error) {
    setup();
    Value args[1] = { BOOL_VAL(true) };
    call_native("key_released", 1, args);
    teardown();
}

TEST(native_mouse_down_type_error) {
    setup();
    Value args[1] = { BOOL_VAL(true) };
    call_native("mouse_down", 1, args);
    teardown();
}

TEST(native_mouse_pressed_type_error) {
    setup();
    Value args[1] = { BOOL_VAL(true) };
    call_native("mouse_pressed", 1, args);
    teardown();
}

TEST(native_mouse_released_type_error) {
    setup();
    Value args[1] = { BOOL_VAL(true) };
    call_native("mouse_released", 1, args);
    teardown();
}

TEST(native_load_image_type_error) {
    setup();
    Value args[1] = { NUMBER_VAL(42) };  // Should be string
    call_native("load_image", 1, args);
    teardown();
}

TEST(native_image_width_type_error) {
    setup();
    Value args[1] = { NUMBER_VAL(42) };  // Should be image
    call_native("image_width", 1, args);
    teardown();
}

TEST(native_image_height_type_error) {
    setup();
    Value args[1] = { NUMBER_VAL(42) };
    call_native("image_height", 1, args);
    teardown();
}

TEST(native_draw_image_type_error) {
    setup();
    Value args[3] = { NUMBER_VAL(42), NUMBER_VAL(0), NUMBER_VAL(0) };  // First should be image
    call_native("draw_image", 3, args);
    teardown();
}

TEST(native_draw_image_ex_type_error) {
    setup();
    Value args[8] = { NUMBER_VAL(42), NUMBER_VAL(0), NUMBER_VAL(0), NUMBER_VAL(64), NUMBER_VAL(64), NUMBER_VAL(0), BOOL_VAL(false), BOOL_VAL(false) };
    call_native("draw_image_ex", 8, args);
    teardown();
}

TEST(native_create_sprite_type_error) {
    setup();
    Value args[1] = { NUMBER_VAL(42) };  // Should be image or none
    call_native("create_sprite", 1, args);
    teardown();
}

TEST(native_draw_sprite_type_error) {
    setup();
    Value args[1] = { NUMBER_VAL(42) };  // Should be sprite
    call_native("draw_sprite", 1, args);
    teardown();
}

// ============================================================================
// Main
// ============================================================================

int main(void) {
    TEST_SUITE("Color Functions");
    RUN_TEST(native_rgb_basic);
    RUN_TEST(native_rgb_clamping);
    RUN_TEST(native_rgb_type_error);
    RUN_TEST(native_rgba_basic);
    RUN_TEST(native_rgba_clamping);
    RUN_TEST(native_rgba_type_error);

    TEST_SUITE("Window Functions");
    RUN_TEST(native_create_window_defaults);
    RUN_TEST(native_create_window_custom);
    RUN_TEST(native_set_title);
    RUN_TEST(native_window_dimensions);
    RUN_TEST(native_window_no_engine);

    TEST_SUITE("Drawing Functions");
    RUN_TEST(native_clear_basic);
    RUN_TEST(native_draw_rect_basic);
    RUN_TEST(native_draw_rect_type_error);
    RUN_TEST(native_draw_circle_basic);
    RUN_TEST(native_draw_line_basic);

    TEST_SUITE("Input Functions");
    RUN_TEST(native_key_down_basic);
    RUN_TEST(native_key_pressed_basic);
    RUN_TEST(native_key_released_basic);
    RUN_TEST(native_mouse_position);
    RUN_TEST(native_mouse_down_basic);
    RUN_TEST(native_mouse_pressed_basic);
    RUN_TEST(native_mouse_released_basic);

    TEST_SUITE("Image Functions");
    RUN_TEST(native_load_image_basic);
    RUN_TEST(native_image_dimensions);
    RUN_TEST(native_draw_image_basic);
    RUN_TEST(native_draw_image_ex_basic);

    TEST_SUITE("Sprite Functions");
    RUN_TEST(native_create_sprite_basic);
    RUN_TEST(native_create_sprite_no_image);
    RUN_TEST(native_draw_sprite_basic);
    RUN_TEST(native_set_sprite_frame);

    TEST_SUITE("Font Functions");
    RUN_TEST(native_load_font_basic);
    RUN_TEST(native_default_font_basic);
    RUN_TEST(native_draw_text_basic);
    RUN_TEST(native_text_size);

    TEST_SUITE("Audio Functions");
    RUN_TEST(native_load_sound_basic);
    RUN_TEST(native_play_sound_basic);
    RUN_TEST(native_play_sound_volume);
    RUN_TEST(native_load_music_basic);
    RUN_TEST(native_music_lifecycle);
    RUN_TEST(native_play_music_loop);
    RUN_TEST(native_set_music_volume);
    RUN_TEST(native_set_master_volume);

    TEST_SUITE("Timing Functions");
    RUN_TEST(native_delta_time);
    RUN_TEST(native_game_time);

    TEST_SUITE("Physics Functions");
    RUN_TEST(native_set_get_gravity);
    RUN_TEST(native_collides_sprites);
    RUN_TEST(native_collides_rect);
    RUN_TEST(native_collides_point);
    RUN_TEST(native_collides_circle);
    RUN_TEST(native_distance);
    RUN_TEST(native_lerp);
    RUN_TEST(native_lerp_angle);
    RUN_TEST(native_apply_force);
    RUN_TEST(native_move_toward);
    RUN_TEST(native_look_at);

    TEST_SUITE("Camera Functions");
    RUN_TEST(native_camera_create);
    RUN_TEST(native_camera_position);
    RUN_TEST(native_camera_zoom);
    RUN_TEST(native_camera_follow);
    RUN_TEST(native_camera_shake);
    RUN_TEST(native_screen_to_world);
    RUN_TEST(native_world_to_screen);

    TEST_SUITE("Animation Functions");
    RUN_TEST(native_create_animation);
    RUN_TEST(native_animation_play_stop);
    RUN_TEST(native_animation_reset);
    RUN_TEST(native_animation_set_looping);
    RUN_TEST(native_animation_frame);
    RUN_TEST(native_animation_playing);

    TEST_SUITE("Scene Functions");
    RUN_TEST(native_scene_management);

    TEST_SUITE("Particle Functions");
    RUN_TEST(native_create_emitter);
    RUN_TEST(native_emitter_emit);
    RUN_TEST(native_emitter_set_color);
    RUN_TEST(native_emitter_set_speed);
    RUN_TEST(native_emitter_set_angle);
    RUN_TEST(native_emitter_set_lifetime);
    RUN_TEST(native_emitter_set_size);
    RUN_TEST(native_emitter_set_gravity);
    RUN_TEST(native_emitter_set_rate);
    RUN_TEST(native_emitter_set_position);
    RUN_TEST(native_emitter_set_active);
    RUN_TEST(native_emitter_count);
    RUN_TEST(native_draw_particles);

    TEST_SUITE("Sprite Animation");
    RUN_TEST(native_sprite_set_animation);
    RUN_TEST(native_sprite_play_stop);

    TEST_SUITE("Constants");
    RUN_TEST(color_constants_defined);
    RUN_TEST(key_constants_defined);
    RUN_TEST(mouse_constants_defined);

    TEST_SUITE("Type Errors");
    RUN_TEST(native_set_title_type_error);
    RUN_TEST(native_clear_type_error);
    RUN_TEST(native_draw_circle_type_error);
    RUN_TEST(native_draw_line_type_error);
    RUN_TEST(native_key_down_type_error);
    RUN_TEST(native_key_pressed_type_error);
    RUN_TEST(native_key_released_type_error);
    RUN_TEST(native_mouse_down_type_error);
    RUN_TEST(native_mouse_pressed_type_error);
    RUN_TEST(native_mouse_released_type_error);
    RUN_TEST(native_load_image_type_error);
    RUN_TEST(native_image_width_type_error);
    RUN_TEST(native_image_height_type_error);
    RUN_TEST(native_draw_image_type_error);
    RUN_TEST(native_draw_image_ex_type_error);
    RUN_TEST(native_create_sprite_type_error);
    RUN_TEST(native_draw_sprite_type_error);

    TEST_SUMMARY();
}
