stered
    void* val;
    ASSERT(table_get_cstr(&vm.globals, "RED", &val));
    Value red = *(Value*)val;
    ASSERT(IS_NUMBER(red));
    ASSERT_EQ((uint32_t)AS_NUMBER(red), COLOR_RED);

    ASSERT(table_get_cstr(&vm.globals, "BLUE", &val));
    Value blue = *(Value*)val;
    ASSERT(IS_NUMBER(blue));
    ASSERT_EQ((uint32_t)AS_NUMBER(blue), COLOR_BLUE);

    engine_shutdown(engine);
    engine_free(engine);
    vm_free(&vm);
}

TEST(native_key_constants) {
    VM vm;
    vm_init(&vm);
    stdlib_init(&vm);

    Engine* engine = engine_new(&vm);
    engine_set(engine);
    engine_init(engine, PAL_BACKEND_MOCK);
    engine_natives_init(&vm);

    // Verify key constants
    void* val;
    ASSERT(table_get_cstr(&vm.globals, "KEY_UP", &val));
    Value key_up = *(Value*)val;
    ASSERT(IS_NUMBER(key_up));
    ASSERT_EQ((int)AS_NUMBER(key_up), PAL_KEY_UP);

    ASSERT(table_get_cstr(&vm.globals, "KEY_SPACE", &val));
    Value key_space = *(Value*)val;
    ASSERT(IS_NUMBER(key_space));
    ASSERT_EQ((int)AS_NUMBER(key_space), PAL_KEY_SPACE);

    engine_shutdown(engine);
    engine_free(engine);
    vm_free(&vm);
}

TEST(native_modifier_key_constants) {
    VM vm;
    vm_init(&vm);
    stdlib_init(&vm);

    Engine* engine = engine_new(&vm);
    engine_set(engine);
    engine_init(engine, PAL_BACKEND_MOCK);
    engine_natives_init(&vm);

    // Verify modifier key constants
    void* val;
    ASSERT(table_get_cstr(&vm.globals, "KEY_SHIFT", &val));
    Value key_shift = *(Value*)val;
    ASSERT(IS_NUMBER(key_shift));
    ASSERT_EQ((int)AS_NUMBER(key_shift), PAL_KEY_LSHIFT);

    ASSERT(table_get_cstr(&vm.globals, "KEY_CTRL", &val));
    Value key_ctrl = *(Value*)val;
    ASSERT(IS_NUMBER(key_ctrl));
    ASSERT_EQ((int)AS_NUMBER(key_ctrl), PAL_KEY_LCTRL);

    ASSERT(table_get_cstr(&vm.globals, "KEY_ALT", &val));
    Value key_alt = *(Value*)val;
    ASSERT(IS_NUMBER(key_alt));
    ASSERT_EQ((int)AS_NUMBER(key_alt), PAL_KEY_LALT);

    // Verify left/right variants
    ASSERT(table_get_cstr(&vm.globals, "KEY_LSHIFT", &val));
    ASSERT(table_get_cstr(&vm.globals, "KEY_RSHIFT", &val));
    ASSERT(table_get_cstr(&vm.globals, "KEY_LCTRL", &val));
    ASSERT(table_get_cstr(&vm.globals, "KEY_RCTRL", &val));
    ASSERT(table_get_cstr(&vm.globals, "KEY_LALT", &val));
    ASSERT(table_get_cstr(&vm.globals, "KEY_RALT", &val));

    engine_shutdown(engine);
    engine_free(engine);
    vm_free(&vm);
}

TEST(native_function_key_constants) {
    VM vm;
    vm_init(&vm);
    stdlib_init(&vm);

    Engine* engine = engine_new(&vm);
    engine_set(engine);
    engine_init(engine, PAL_BACKEND_MOCK);
    engine_natives_init(&vm);

    // Verify function key constants
    void* val;
    ASSERT(table_get_cstr(&vm.globals, "KEY_F1", &val));
    Value key_f1 = *(Value*)val;
    ASSERT(IS_NUMBER(key_f1));
    ASSERT_EQ((int)AS_NUMBER(key_f1), PAL_KEY_F1);

    ASSERT(table_get_cstr(&vm.globals, "KEY_F12", &val));
    Value key_f12 = *(Value*)val;
    ASSERT(IS_NUMBER(key_f12));
    ASSERT_EQ((int)AS_NUMBER(key_f12), PAL_KEY_F12);

    // Verify backspace
    ASSERT(table_get_cstr(&vm.globals, "KEY_BACKSPACE", &val));
    Value key_bs = *(Value*)val;
    ASSERT(IS_NUMBER(key_bs));
    ASSERT_EQ((int)AS_NUMBER(key_bs), PAL_KEY_BACKSPACE);

    engine_shutdown(engine);
    engine_free(engine);
    vm_free(&vm);
}

TEST(native_functions_registered) {
    VM vm;
    vm_init(&vm);
    stdlib_init(&vm);

    Engine* engine = engine_new(&vm);
    engine_set(engine);
    engine_init(engine, PAL_BACKEND_MOCK);
    engine_natives_init(&vm);

    // Check that key functions are registered
    void* val;
    ASSERT(table_get_cstr(&vm.globals, "rgb", &val));
    ASSERT(table_get_cstr(&vm.globals, "rgba", &val));
    ASSERT(table_get_cstr(&vm.globals, "clear", &val));
    ASSERT(table_get_cstr(&vm.globals, "draw_rect", &val));
    ASSERT(table_get_cstr(&vm.globals, "draw_circle", &val));
    ASSERT(table_get_cstr(&vm.globals, "draw_line", &val));
    ASSERT(table_get_cstr(&vm.globals, "key_down", &val));
    ASSERT(table_get_cstr(&vm.globals, "create_window", &val));

    engine_shutdown(engine);
    engine_free(engine);
    vm_free(&vm);
}

TEST(native_input_functions_registered) {
    VM vm;
    vm_init(&vm);
    stdlib_init(&vm);

    Engine* engine = engine_new(&vm);
    engine_set(engine);
    engine_init(engine, PAL_BACKEND_MOCK);
    engine_natives_init(&vm);

    // Verify all input functions are registered
    void* val;

    // Keyboard functions
    ASSERT(table_get_cstr(&vm.globals, "key_down", &val));
    ASSERT(table_get_cstr(&vm.globals, "key_pressed", &val));
    ASSERT(table_get_cstr(&vm.globals, "key_released", &val));

    // Mouse functions
    ASSERT(table_get_cstr(&vm.globals, "mouse_x", &val));
    ASSERT(table_get_cstr(&vm.globals, "mouse_y", &val));
    ASSERT(table_get_cstr(&vm.globals, "mouse_down", &val));
    ASSERT(table_get_cstr(&vm.globals, "mouse_pressed", &val));
    ASSERT(table_get_cstr(&vm.globals, "mouse_released", &val));

    // Mouse button constants
    ASSERT(table_get_cstr(&vm.globals, "MOUSE_LEFT", &val));
    ASSERT(table_get_cstr(&vm.globals, "MOUSE_MIDDLE", &val));
    ASSERT(table_get_cstr(&vm.globals, "MOUSE_RIGHT", &val));

    engine_shutdown(engine);
    engine_free(engine);
    vm_free(&vm);
}

TEST(native_audio_functions_registered) {
    VM vm;
    vm_init(&vm);
    stdlib_init(&vm);

    Engine* engine = engine_new(&vm);
    engine_set(engine);
    engine_init(engine, PAL_BACKEND_MOCK);
    engine_natives_init(&vm);

    // Verify all audio functions are registered
    void* val;

    // Sound functions
    ASSERT(table_get_cstr(&vm.globals, "load_sound", &val));
    ASSERT(table_get_cstr(&vm.globals, "play_sound", &val));
    ASSERT(table_get_cstr(&vm.globals, "play_sound_volume", &val));

    // Music functions
    ASSERT(table_get_cstr(&vm.globals, "load_music", &val));
    ASSERT(table_get_cstr(&vm.globals, "play_music", &val));
    ASSERT(table_get_cstr(&vm.globals, "play_music_loop", &val));
    ASSERT(table_get_cstr(&vm.globals, "pause_music", &val));
    ASSERT(table_get_cstr(&vm.globals, "resume_music", &val));
    ASSERT(table_get_cstr(&vm.globals, "stop_music", &val));
    ASSERT(table_get_cstr(&vm.globals, "set_music_volume", &val));
    ASSERT(table_get_cstr(&vm.globals, "music_playing", &val));

    // Master volume
    ASSERT(table_get_cstr(&vm.globals, "set_master_volume", &val));

    engine_shutdown(engine);
    engine_free(engine);
    vm_free(&vm);
}

// ============================================================================
// Engine Stop Tests
// ============================================================================

TEST(engine_stop) {
    VM vm;
    vm_init(&vm);

    Engine* engine = engine_new(&vm);
    engine->running = true;

    engine_stop(engine);
    ASSERT(!engine->running);

    engine_free(engine);
    vm_free(&vm);
}

// ============================================================================
// Main
// ============================================================================

int main(void) {
    TEST_SUITE("Color Functions");
    RUN_TEST(color_pack_unpack);
    RUN_TEST(color_constants);
    RUN_TEST(color_pack_extremes);

    TEST_SUITE("Engine Lifecycle");
    RUN_TEST(engine_new_free);
    RUN_TEST(engine_init_shutdown);
    RUN_TEST(engine_global_access);

    TEST_SUITE("Window Management");
    RUN_TEST(engine_create_window);
    RUN_TEST(engine_set_title);

    TEST_SUITE("Callback Detection");
    RUN_TEST(engine_no_callbacks);

    TEST_SUITE("Native Functions");
    RUN_TEST(native_rgb_rgba);
    RUN_TEST(native_key_constants);
    RUN_TEST(native_modifier_key_constants);
    RUN_TEST(native_function_key_constants);
    RUN_TEST(native_functions_registered);
    RUN_TEST(native_input_functions_registered);
    RUN_TEST(native_audio_functions_registered);

    TEST_SUITE("Engine Control");
    RUN_TEST(engine_stop);

    TEST_SUMMARY();
}
// Tests for the Game Engine Core

#define PAL_MOCK_ENABLED
#include "../test_framework.h"
#include "engine/engine.h"
#include "engine/engine_natives.h"
#include "runtime/stdlib.h"
#include "vm/vm.h"
#include "pal/pal.h"

// ============================================================================
// Color Tests
// ============================================================================

TEST(color_pack_unpack) {
    uint32_t color = pack_color(255, 128, 64, 200);
    uint8_t r, g, b, a;
    unpack_color(color, &r, &g, &b, &a);

    ASSERT_EQ(r, 255);
    ASSERT_EQ(g, 128);
    ASSERT_EQ(b, 64);
    ASSERT_EQ(a, 200);
}

TEST(color_constants) {
    ASSERT_EQ(COLOR_RED, 0xFF0000FFu);
    ASSERT_EQ(COLOR_GREEN, 0x00FF00FFu);
    ASSERT_EQ(COLOR_BLUE, 0x0000FFFFu);
    ASSERT_EQ(COLOR_WHITE, 0xFFFFFFFFu);
    ASSERT_EQ(COLOR_BLACK, 0x000000FFu);
}

TEST(color_pack_extremes) {
    // All zeros
    uint32_t color = pack_color(0, 0, 0, 0);
    ASSERT_EQ(color, 0x00000000u);

    // All 255s
    color = pack_color(255, 255, 255, 255);
    ASSERT_EQ(color, 0xFFFFFFFFu);
}

// ============================================================================
// Engine Lifecycle Tests
// ============================================================================

TEST(engine_new_free) {
    VM vm;
    vm_init(&vm);

    Engine* engine = engine_new(&vm);
    ASSERT_NOT_NULL(engine);
    ASSERT_EQ(engine->vm, &vm);
    ASSERT_NULL(engine->window);
    ASSERT_NULL(engine->on_start);
    ASSERT_NULL(engine->on_update);
    ASSERT_NULL(engine->on_draw);
    ASSERT(!engine->running);

    engine_free(engine);
    vm_free(&vm);
}

TEST(engine_init_shutdown) {
    VM vm;
    vm_init(&vm);

    Engine* engine = engine_new(&vm);
    ASSERT_NOT_NULL(engine);

    ASSERT(engine_init(engine, PAL_BACKEND_MOCK));
    ASSERT_EQ(pal_get_backend(), PAL_BACKEND_MOCK);

    engine_shutdown(engine);
    engine_free(engine);
    vm_free(&vm);
}

TEST(engine_global_access) {
    VM vm;
    vm_init(&vm);

    Engine* engine = engine_new(&vm);
    engine_set(engine);

    ASSERT_EQ(engine_get(), engine);

    engine_set(NULL);
    ASSERT_NULL(engine_get());

    engine_free(engine);
    vm_free(&vm);
}

// ============================================================================
// Window Management Tests
// ============================================================================

TEST(engine_create_window) {
    VM vm;
    vm_init(&vm);

    Engine* engine = engine_new(&vm);
    engine_init(engine, PAL_BACKEND_MOCK);

    ASSERT(engine_create_window(engine, "Test Game", 1024, 768));
    ASSERT(engine->window_created);
    ASSERT_NOT_NULL(engine->window);

    ASSERT_EQ(engine_get_width(engine), 1024);
    ASSERT_EQ(engine_get_height(engine), 768);

    engine_shutdown(engine);
    engine_free(engine);
    vm_free(&vm);
}

TEST(engine_set_title) {
    VM vm;
    vm_init(&vm);

    Engine* engine = engine_new(&vm);
    engine_init(engine, PAL_BACKEND_MOCK);
    engine_create_window(engine, "Original", 800, 600);

    pal_mock_clear_calls();
    engine_set_title(engine, "New Title");

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

    engine_shutdown(engine);
    engine_free(engine);
    vm_free(&vm);
}

// ============================================================================
// Callback Detection Tests
// ============================================================================

TEST(engine_no_callbacks) {
    VM vm;
    vm_init(&vm);

    Engine* engine = engine_new(&vm);
    engine_detect_callbacks(engine);

    ASSERT(!engine_has_callbacks(engine));
    ASSERT_NULL(engine->on_start);
    ASSERT_NULL(engine->on_update);
    ASSERT_NULL(engine->on_draw);
    ASSERT_NULL(engine->on_key_down);
    ASSERT_NULL(engine->on_key_up);
    ASSERT_NULL(engine->on_mouse_click);
    ASSERT_NULL(engine->on_mouse_move);

    engine_free(engine);
    vm_free(&vm);
}

// ============================================================================
// Native Function Tests
// ============================================================================

TEST(native_rgb_rgba) {
    VM vm;
    vm_init(&vm);
    stdlib_init(&vm);

    Engine* engine = engine_new(&vm);
    engine_set(engine);
    engine_init(engine, PAL_BACKEND_MOCK);
    engine_natives_init(&vm);

    // Verify color constants are regi