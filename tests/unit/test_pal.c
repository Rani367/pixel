---------------------

TEST(time_functions) {
    ASSERT(pal_init(PAL_BACKEND_MOCK));
    pal_mock_clear_calls();

    double t1 = pal_time();
    ASSERT(t1 >= 0.0);

    // Sleep doesn't actually wait in mock, just records the call
    pal_sleep(0.1);

    int count;
    const PalMockCall* calls = pal_mock_get_calls(&count);
    bool found = false;
    for (int i = 0; i < count; i++) {
        if (strcmp(calls[i].function, "pal_sleep") == 0) {
            found = true;
            break;
        }
    }
    ASSERT(found);

    pal_quit();
}

// -----------------------------------------------------------------------------
// Call recording tests
// -----------------------------------------------------------------------------

TEST(call_recording) {
    ASSERT(pal_init(PAL_BACKEND_MOCK));
    pal_mock_clear_calls();

    int count;
    const PalMockCall* calls = pal_mock_get_calls(&count);
    ASSERT_EQ(count, 0);

    PalWindow* window = pal_window_create("Test", 800, 600);
    pal_window_clear(window, 0, 0, 0);
    pal_window_present(window);

    calls = pal_mock_get_calls(&count);
    ASSERT_EQ(count, 3);
    ASSERT_STR_EQ(calls[0].function, "pal_window_create");
    ASSERT_STR_EQ(calls[1].function, "pal_window_clear");
    ASSERT_STR_EQ(calls[2].function, "pal_window_present");

    pal_mock_clear_calls();
    calls = pal_mock_get_calls(&count);
    ASSERT_EQ(count, 0);

    pal_window_destroy(window);
    pal_quit();
}

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------

int main(void) {
    TEST_SUITE("PAL Initialization");
    RUN_TEST(init_and_quit);
    RUN_TEST(double_init);

    TEST_SUITE("PAL Window");
    RUN_TEST(window_create_destroy);
    RUN_TEST(window_size);
    RUN_TEST(window_clear);
    RUN_TEST(window_present);

    TEST_SUITE("PAL Rendering");
    RUN_TEST(draw_primitives);

    TEST_SUITE("PAL Textures");
    RUN_TEST(texture_load_destroy);
    RUN_TEST(texture_draw);

    TEST_SUITE("PAL Input");
    RUN_TEST(keyboard_input);
    RUN_TEST(mouse_input);
    RUN_TEST(quit_request);

    TEST_SUITE("PAL Audio");
    RUN_TEST(sound_effects);
    RUN_TEST(music);

    TEST_SUITE("PAL Time");
    RUN_TEST(time_functions);

    TEST_SUITE("PAL Mock");
    RUN_TEST(call_recording);

    TEST_SUMMARY();
}
--------------------------------------------------------

TEST(keyboard_input) {
    ASSERT(pal_init(PAL_BACKEND_MOCK));

    // Initially no keys pressed
    ASSERT(!pal_key_down(PAL_KEY_SPACE));
    ASSERT(!pal_key_pressed(PAL_KEY_SPACE));
    ASSERT(!pal_key_released(PAL_KEY_SPACE));

    // Establish baseline with poll_events, then simulate key press
    pal_poll_events();  // prev = false
    pal_mock_set_key(PAL_KEY_SPACE, true);  // down = true

    // Now key should show as pressed (down=true, prev=false)
    ASSERT(pal_key_down(PAL_KEY_SPACE));
    ASSERT(pal_key_pressed(PAL_KEY_SPACE));
    ASSERT(!pal_key_released(PAL_KEY_SPACE));

    // Next frame, still held
    pal_poll_events();  // prev = true, down = true
    ASSERT(pal_key_down(PAL_KEY_SPACE));
    ASSERT(!pal_key_pressed(PAL_KEY_SPACE));  // Not "just pressed"
    ASSERT(!pal_key_released(PAL_KEY_SPACE));

    // Release key
    pal_poll_events();  // prev = true
    pal_mock_set_key(PAL_KEY_SPACE, false);  // down = false

    ASSERT(!pal_key_down(PAL_KEY_SPACE));
    ASSERT(!pal_key_pressed(PAL_KEY_SPACE));
    ASSERT(pal_key_released(PAL_KEY_SPACE));

    pal_quit();
}

TEST(mouse_input) {
    ASSERT(pal_init(PAL_BACKEND_MOCK));

    // Set mouse position
    pal_mock_set_mouse_position(400, 300);
    int x, y;
    pal_mouse_position(&x, &y);
    ASSERT_EQ(x, 400);
   // Tests for the Platform Abstraction Layer (mock backend)

#define PAL_MOCK_ENABLED
#include "../test_framework.h"
#include "pal/pal.h"

// -----------------------------------------------------------------------------
// Initialization tests
// -----------------------------------------------------------------------------

TEST(init_and_quit) {
    ASSERT(pal_init(PAL_BACKEND_MOCK));
    ASSERT_EQ(pal_get_backend(), PAL_BACKEND_MOCK);
    pal_quit();
}

TEST(double_init) {
    ASSERT(pal_init(PAL_BACKEND_MOCK));
    ASSERT(pal_init(PAL_BACKEND_MOCK));  // Should succeed (reinit)
    pal_quit();
}

// -----------------------------------------------------------------------------
// Window tests
// -----------------------------------------------------------------------------

TEST(window_create_destroy) {
    ASSERT(pal_init(PAL_BACKEND_MOCK));
    pal_mock_clear_calls();

    PalWindow* window = pal_window_create("Test Window", 800, 600);
    ASSERT_NOT_NULL(window);

    int count;
    const PalMockCall* calls = pal_mock_get_calls(&count);
    ASSERT(count >= 1);
    ASSERT_STR_EQ(calls[0].function, "pal_window_create");

    pal_window_destroy(window);
    pal_quit();
}

TEST(window_size) {
    ASSERT(pal_init(PAL_BACKEND_MOCK));

    PalWindow* window = pal_window_create("Test", 1024, 768);
    ASSERT_NOT_NULL(window);

    int width, height;
    pal_window_get_size(window, &width, &height);
    ASSERT_EQ(width, 1024);
    ASSERT_EQ(height, 768);

    pal_window_destroy(window);
    pal_quit();
}

TEST(window_clear) {
    ASSERT(pal_init(PAL_BACKEND_MOCK));
    pal_mock_clear_calls();

    PalWindow* window = pal_window_create("Test", 800, 600);
    pal_window_clear(window, 100, 150, 200);

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

    pal_window_destroy(window);
    pal_quit();
}

TEST(window_present) {
    ASSERT(pal_init(PAL_BACKEND_MOCK));
    pal_mock_clear_calls();

    PalWindow* window = pal_window_create("Test", 800, 600);
    pal_window_present(window);

    int count;
    const PalMockCall* calls = pal_mock_get_calls(&count);
    bool found = false;
    for (int i = 0; i < count; i++) {
        if (strcmp(calls[i].function, "pal_window_present") == 0) {
            found = true;
            break;
        }
    }
    ASSERT(found);

    pal_window_destroy(window);
    pal_quit();
}

// -----------------------------------------------------------------------------
// Rendering tests
// -----------------------------------------------------------------------------

TEST(draw_primitives) {
    ASSERT(pal_init(PAL_BACKEND_MOCK));
    PalWindow* window = pal_window_create("Test", 800, 600);
    pal_mock_clear_calls();

    pal_draw_rect(window, 10, 20, 100, 50, 255, 0, 0, 255);
    pal_draw_rect_outline(window, 10, 20, 100, 50, 0, 255, 0, 255);
    pal_draw_line(window, 0, 0, 100, 100, 0, 0, 255, 255);
    pal_draw_circle(window, 400, 300, 50, 255, 255, 0, 255);
    pal_draw_circle_outline(window, 400, 300, 60, 255, 0, 255, 255);

    int count;
    pal_mock_get_calls(&count);
    ASSERT_EQ(count, 5);

    pal_window_destroy(window);
    pal_quit();
}

// -----------------------------------------------------------------------------
// Texture tests
// -----------------------------------------------------------------------------

TEST(texture_load_destroy) {
    ASSERT(pal_init(PAL_BACKEND_MOCK));
    PalWindow* window = pal_window_create("Test", 800, 600);
    pal_mock_clear_calls();

    PalTexture* texture = pal_texture_load(window, "test.png");
    ASSERT_NOT_NULL(texture);

    int width, height;
    pal_texture_get_size(texture, &width, &height);
    ASSERT_EQ(width, 64);   // Mock default
    ASSERT_EQ(height, 64);

    pal_texture_destroy(texture);

    pal_window_destroy(window);
    pal_quit();
}

TEST(texture_draw) {
    ASSERT(pal_init(PAL_BACKEND_MOCK));
    PalWindow* window = pal_window_create("Test", 800, 600);
    PalTexture* texture = pal_texture_load(window, "test.png");
    pal_mock_clear_calls();

    pal_draw_texture(window, texture, 100, 100, 64, 64);
    pal_draw_texture_ex(window, texture, 200, 200, 64, 64, 45.0, 32, 32, false, true);
    pal_draw_texture_region(window, texture, 0, 0, 32, 32, 300, 300, 64, 64);

    int count;
    pal_mock_get_calls(&count);
    ASSERT_EQ(count, 3);

    pal_texture_destroy(texture);
    pal_window_destroy(window);
    pal_quit();
}

// -----------------------------------------------------------------------------
// Input tests
// --------------------- ASSERT_EQ(y, 300);

    // Mouse button - establish baseline first
    ASSERT(!pal_mouse_down(PAL_MOUSE_LEFT));
    pal_poll_events();  // prev = false
    pal_mock_set_mouse_button(PAL_MOUSE_LEFT, true);  // down = true

    ASSERT(pal_mouse_down(PAL_MOUSE_LEFT));
    ASSERT(pal_mouse_pressed(PAL_MOUSE_LEFT));

    pal_poll_events();  // prev = true, down = true
    ASSERT(pal_mouse_down(PAL_MOUSE_LEFT));
    ASSERT(!pal_mouse_pressed(PAL_MOUSE_LEFT));

    pal_poll_events();  // prev = true
    pal_mock_set_mouse_button(PAL_MOUSE_LEFT, false);  // down = false
    ASSERT(!pal_mouse_down(PAL_MOUSE_LEFT));
    ASSERT(pal_mouse_released(PAL_MOUSE_LEFT));

    pal_quit();
}

TEST(quit_request) {
    ASSERT(pal_init(PAL_BACKEND_MOCK));

    ASSERT(!pal_should_quit());

    pal_mock_set_quit(true);
    ASSERT(pal_should_quit());

    pal_mock_set_quit(false);
    ASSERT(!pal_should_quit());

    pal_quit();
}

// -----------------------------------------------------------------------------
// Audio tests
// -----------------------------------------------------------------------------

TEST(sound_effects) {
    ASSERT(pal_init(PAL_BACKEND_MOCK));
    pal_mock_clear_calls();

    PalSound* sound = pal_sound_load("test.wav");
    ASSERT_NOT_NULL(sound);

    pal_sound_play(sound);
    pal_sound_play_volume(sound, 0.5f);

    int count;
    (void)pal_mock_get_calls(&count);
    ASSERT(count >= 3);  // load, play, play_volume

    pal_sound_destroy(sound);
    pal_quit();
}

TEST(music) {
    ASSERT(pal_init(PAL_BACKEND_MOCK));
    pal_mock_clear_calls();

    PalMusic* music = pal_music_load("test.ogg");
    ASSERT_NOT_NULL(music);

    ASSERT(!pal_music_is_playing());

    pal_music_play(music, true);
    ASSERT(pal_music_is_playing());

    pal_music_pause();
    ASSERT(!pal_music_is_playing());

    pal_music_resume();
    ASSERT(pal_music_is_playing());

    pal_music_set_volume(0.8f);

    pal_music_stop();
    ASSERT(!pal_music_is_playing());

    pal_music_destroy(music);
    pal_quit();
}

// -----------------------------------------------------------------------------
// Time tests
// --------------------------------------------------------