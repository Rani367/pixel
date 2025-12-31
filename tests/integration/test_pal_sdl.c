// Tests for the SDL2 PAL backend using headless drivers
// These tests run with SDL_VIDEODRIVER=dummy and SDL_AUDIODRIVER=dummy

#ifdef PAL_USE_SDL2

#include "../test_framework.h"
#include "pal/pal.h"
#include <SDL.h>
#include <string.h>

// =============================================================================
// Setup/Teardown
// =============================================================================

static void setup_headless(void) {
    // Set hints for headless operation BEFORE any SDL calls
    SDL_SetHint(SDL_HINT_VIDEODRIVER, "dummy");
    SDL_SetHint(SDL_HINT_AUDIODRIVER, "dummy");
}

// =============================================================================
// Initialization Tests
// =============================================================================

TEST(sdl_init_with_dummy_drivers) {
    setup_headless();
    ASSERT(pal_init(PAL_BACKEND_SDL2));
    ASSERT_EQ(pal_get_backend(), PAL_BACKEND_SDL2);
    pal_quit();
}

TEST(sdl_quit_cleanup) {
    setup_headless();
    ASSERT(pal_init(PAL_BACKEND_SDL2));
    pal_quit();
    // Reinit should work after quit
    ASSERT(pal_init(PAL_BACKEND_SDL2));
    pal_quit();
}

TEST(sdl_double_init) {
    setup_headless();
    ASSERT(pal_init(PAL_BACKEND_SDL2));
    ASSERT(pal_init(PAL_BACKEND_SDL2));  // Should succeed (already initialized)
    pal_quit();
}

// =============================================================================
// Window Tests
// =============================================================================

TEST(sdl_window_create) {
    setup_headless();
    ASSERT(pal_init(PAL_BACKEND_SDL2));

    PalWindow* window = pal_window_create("Test Window", 800, 600);
    ASSERT_NOT_NULL(window);

    pal_window_destroy(window);
    pal_quit();
}

TEST(sdl_window_destroy_null) {
    setup_headless();
    ASSERT(pal_init(PAL_BACKEND_SDL2));

    // Should not crash with NULL
    pal_window_destroy(NULL);

    pal_quit();
}

TEST(sdl_window_get_size) {
    setup_headless();
    ASSERT(pal_init(PAL_BACKEND_SDL2));

    PalWindow* window = pal_window_create("Test", 1024, 768);
    ASSERT_NOT_NULL(window);

    int width, height;
    pal_window_get_size(window, &width, &height);
    ASSERT_EQ(width, 1024);
    ASSERT_EQ(height, 768);

    pal_window_destroy(window);
    pal_quit();
}

TEST(sdl_window_get_size_null) {
    setup_headless();
    ASSERT(pal_init(PAL_BACKEND_SDL2));

    int width = 999, height = 999;
    pal_window_get_size(NULL, &width, &height);
    ASSERT_EQ(width, 0);
    ASSERT_EQ(height, 0);

    pal_quit();
}

TEST(sdl_window_set_title) {
    setup_headless();
    ASSERT(pal_init(PAL_BACKEND_SDL2));

    PalWindow* window = pal_window_create("Initial", 640, 480);
    ASSERT_NOT_NULL(window);

    pal_window_set_title(window, "New Title");
    // No crash = success (can't easily verify title in headless)

    pal_window_set_title(NULL, "Test");  // Should not crash
    pal_window_set_title(window, NULL);  // Should handle NULL title

    pal_window_destroy(window);
    pal_quit();
}

TEST(sdl_window_clear) {
    setup_headless();
    ASSERT(pal_init(PAL_BACKEND_SDL2));

    PalWindow* window = pal_window_create("Test", 640, 480);
    ASSERT_NOT_NULL(window);

    pal_window_clear(window, 100, 150, 200);
    pal_window_clear(NULL, 0, 0, 0);  // Should not crash

    pal_window_destroy(window);
    pal_quit();
}

TEST(sdl_window_present) {
    setup_headless();
    ASSERT(pal_init(PAL_BACKEND_SDL2));

    PalWindow* window = pal_window_create("Test", 640, 480);
    ASSERT_NOT_NULL(window);

    pal_window_present(window);
    pal_window_present(NULL);  // Should not crash

    pal_window_destroy(window);
    pal_quit();
}

// =============================================================================
// Drawing Tests
// =============================================================================

TEST(sdl_draw_rect) {
    setup_headless();
    ASSERT(pal_init(PAL_BACKEND_SDL2));

    PalWindow* window = pal_window_create("Test", 640, 480);
    ASSERT_NOT_NULL(window);

    pal_draw_rect(window, 10, 20, 100, 50, 255, 0, 0, 255);
    pal_draw_rect(window, 0, 0, 640, 480, 0, 255, 0, 128);  // Semi-transparent
    pal_draw_rect(NULL, 0, 0, 100, 100, 0, 0, 0, 255);  // Should not crash

    pal_window_destroy(window);
    pal_quit();
}

TEST(sdl_draw_rect_outline) {
    setup_headless();
    ASSERT(pal_init(PAL_BACKEND_SDL2));

    PalWindow* window = pal_window_create("Test", 640, 480);
    ASSERT_NOT_NULL(window);

    pal_draw_rect_outline(window, 10, 20, 100, 50, 255, 0, 0, 255);
    pal_draw_rect_outline(NULL, 0, 0, 100, 100, 0, 0, 0, 255);

    pal_window_destroy(window);
    pal_quit();
}

TEST(sdl_draw_line) {
    setup_headless();
    ASSERT(pal_init(PAL_BACKEND_SDL2));

    PalWindow* window = pal_window_create("Test", 640, 480);
    ASSERT_NOT_NULL(window);

    pal_draw_line(window, 0, 0, 640, 480, 255, 255, 0, 255);
    pal_draw_line(window, -10, -10, 700, 500, 0, 0, 255, 255);  // Off-screen
    pal_draw_line(NULL, 0, 0, 100, 100, 0, 0, 0, 255);

    pal_window_destroy(window);
    pal_quit();
}

TEST(sdl_draw_circle) {
    setup_headless();
    ASSERT(pal_init(PAL_BACKEND_SDL2));

    PalWindow* window = pal_window_create("Test", 640, 480);
    ASSERT_NOT_NULL(window);

    pal_draw_circle(window, 320, 240, 100, 255, 0, 255, 255);
    pal_draw_circle(window, 0, 0, 50, 0, 255, 255, 128);  // Partially off-screen
    pal_draw_circle(window, 320, 240, 0, 255, 255, 255, 255);  // Zero radius
    pal_draw_circle(NULL, 0, 0, 100, 0, 0, 0, 255);

    pal_window_destroy(window);
    pal_quit();
}

TEST(sdl_draw_circle_outline) {
    setup_headless();
    ASSERT(pal_init(PAL_BACKEND_SDL2));

    PalWindow* window = pal_window_create("Test", 640, 480);
    ASSERT_NOT_NULL(window);

    pal_draw_circle_outline(window, 320, 240, 100, 255, 255, 0, 255);
    pal_draw_circle_outline(NULL, 0, 0, 100, 0, 0, 0, 255);

    pal_window_destroy(window);
    pal_quit();
}

TEST(sdl_draw_edge_cases) {
    setup_headless();
    ASSERT(pal_init(PAL_BACKEND_SDL2));

    PalWindow* window = pal_window_create("Test", 640, 480);
    ASSERT_NOT_NULL(window);

    // Zero-size shapes
    pal_draw_rect(window, 100, 100, 0, 0, 255, 0, 0, 255);
    pal_draw_rect(window, 100, 100, -10, -10, 255, 0, 0, 255);

    // Large coordinates
    pal_draw_rect(window, 10000, 10000, 100, 100, 255, 0, 0, 255);
    pal_draw_line(window, -1000, -1000, 10000, 10000, 0, 255, 0, 255);

    // Negative coordinates
    pal_draw_rect(window, -50, -50, 100, 100, 0, 0, 255, 255);

    pal_window_destroy(window);
    pal_quit();
}

// =============================================================================
// Texture Tests
// =============================================================================

TEST(sdl_texture_load_missing) {
    setup_headless();
    ASSERT(pal_init(PAL_BACKEND_SDL2));

    PalWindow* window = pal_window_create("Test", 640, 480);
    ASSERT_NOT_NULL(window);

    // Loading missing file should return NULL
    PalTexture* texture = pal_texture_load(window, "nonexistent_file.png");
    ASSERT_NULL(texture);

    pal_window_destroy(window);
    pal_quit();
}

TEST(sdl_texture_destroy_null) {
    setup_headless();
    ASSERT(pal_init(PAL_BACKEND_SDL2));

    // Should not crash with NULL
    pal_texture_destroy(NULL);

    pal_quit();
}

TEST(sdl_texture_get_size_null) {
    setup_headless();
    ASSERT(pal_init(PAL_BACKEND_SDL2));

    int width = 999, height = 999;
    pal_texture_get_size(NULL, &width, &height);
    ASSERT_EQ(width, 0);
    ASSERT_EQ(height, 0);

    pal_quit();
}

TEST(sdl_draw_texture_null) {
    setup_headless();
    ASSERT(pal_init(PAL_BACKEND_SDL2));

    PalWindow* window = pal_window_create("Test", 640, 480);
    ASSERT_NOT_NULL(window);

    // Should not crash with NULL texture
    pal_draw_texture(window, NULL, 0, 0, 64, 64);
    pal_draw_texture(NULL, NULL, 0, 0, 64, 64);

    pal_window_destroy(window);
    pal_quit();
}

TEST(sdl_draw_texture_ex_null) {
    setup_headless();
    ASSERT(pal_init(PAL_BACKEND_SDL2));

    PalWindow* window = pal_window_create("Test", 640, 480);
    ASSERT_NOT_NULL(window);

    // Should not crash with NULL
    pal_draw_texture_ex(window, NULL, 0, 0, 64, 64, 45.0, 32, 32, false, false);
    pal_draw_texture_ex(NULL, NULL, 0, 0, 64, 64, 0.0, 0, 0, true, true);

    pal_window_destroy(window);
    pal_quit();
}

TEST(sdl_draw_texture_region_null) {
    setup_headless();
    ASSERT(pal_init(PAL_BACKEND_SDL2));

    PalWindow* window = pal_window_create("Test", 640, 480);
    ASSERT_NOT_NULL(window);

    // Should not crash with NULL
    pal_draw_texture_region(window, NULL, 0, 0, 32, 32, 100, 100, 64, 64);
    pal_draw_texture_region(NULL, NULL, 0, 0, 32, 32, 0, 0, 64, 64);

    pal_window_destroy(window);
    pal_quit();
}

// =============================================================================
// Input Tests
// =============================================================================

TEST(sdl_poll_events) {
    setup_headless();
    ASSERT(pal_init(PAL_BACKEND_SDL2));

    // Poll events - should not hang or crash
    for (int i = 0; i < 10; i++) {
        pal_poll_events();
    }

    pal_quit();
}

TEST(sdl_should_quit_initial) {
    setup_headless();
    ASSERT(pal_init(PAL_BACKEND_SDL2));

    // Initially should not be quit
    ASSERT(!pal_should_quit());

    pal_quit();
}

TEST(sdl_key_down_initial) {
    setup_headless();
    ASSERT(pal_init(PAL_BACKEND_SDL2));

    // No keys should be pressed initially
    ASSERT(!pal_key_down(PAL_KEY_A));
    ASSERT(!pal_key_down(PAL_KEY_SPACE));
    ASSERT(!pal_key_down(PAL_KEY_ESCAPE));
    ASSERT(!pal_key_down(PAL_KEY_LEFT));
    ASSERT(!pal_key_down(PAL_KEY_RIGHT));

    pal_quit();
}

TEST(sdl_key_pressed_released) {
    setup_headless();
    ASSERT(pal_init(PAL_BACKEND_SDL2));

    pal_poll_events();

    // No keys pressed/released initially
    ASSERT(!pal_key_pressed(PAL_KEY_A));
    ASSERT(!pal_key_released(PAL_KEY_A));

    pal_quit();
}

TEST(sdl_mouse_position) {
    setup_headless();
    ASSERT(pal_init(PAL_BACKEND_SDL2));

    int x, y;
    pal_mouse_position(&x, &y);
    // In headless mode, position should be 0,0 or some default
    // Just verify it doesn't crash

    pal_quit();
}

TEST(sdl_mouse_buttons) {
    setup_headless();
    ASSERT(pal_init(PAL_BACKEND_SDL2));

    pal_poll_events();

    // No mouse buttons pressed initially
    ASSERT(!pal_mouse_down(PAL_MOUSE_LEFT));
    ASSERT(!pal_mouse_down(PAL_MOUSE_MIDDLE));
    ASSERT(!pal_mouse_down(PAL_MOUSE_RIGHT));
    ASSERT(!pal_mouse_pressed(PAL_MOUSE_LEFT));
    ASSERT(!pal_mouse_released(PAL_MOUSE_LEFT));

    pal_quit();
}

TEST(sdl_input_state_between_frames) {
    setup_headless();
    ASSERT(pal_init(PAL_BACKEND_SDL2));

    // Simulate multiple frames
    for (int i = 0; i < 5; i++) {
        pal_poll_events();
        (void)pal_key_down(PAL_KEY_A);
        (void)pal_key_pressed(PAL_KEY_A);
        (void)pal_key_released(PAL_KEY_A);
        (void)pal_mouse_down(PAL_MOUSE_LEFT);
    }

    pal_quit();
}

// =============================================================================
// Audio Tests
// =============================================================================

TEST(sdl_sound_load_missing) {
    setup_headless();
    ASSERT(pal_init(PAL_BACKEND_SDL2));

    // Loading missing file should return NULL
    PalSound* sound = pal_sound_load("nonexistent_sound.wav");
    ASSERT_NULL(sound);

    pal_quit();
}

TEST(sdl_sound_destroy_null) {
    setup_headless();
    ASSERT(pal_init(PAL_BACKEND_SDL2));

    // Should not crash
    pal_sound_destroy(NULL);

    pal_quit();
}

TEST(sdl_sound_play_null) {
    setup_headless();
    ASSERT(pal_init(PAL_BACKEND_SDL2));

    // Should not crash with NULL
    pal_sound_play(NULL);
    pal_sound_play_volume(NULL, 0.5f);

    pal_quit();
}

TEST(sdl_music_load_missing) {
    setup_headless();
    ASSERT(pal_init(PAL_BACKEND_SDL2));

    // Loading missing file should return NULL
    PalMusic* music = pal_music_load("nonexistent_music.ogg");
    ASSERT_NULL(music);

    pal_quit();
}

TEST(sdl_music_destroy_null) {
    setup_headless();
    ASSERT(pal_init(PAL_BACKEND_SDL2));

    // Should not crash
    pal_music_destroy(NULL);

    pal_quit();
}

TEST(sdl_music_controls_null) {
    setup_headless();
    ASSERT(pal_init(PAL_BACKEND_SDL2));

    // Should not crash with NULL
    pal_music_play(NULL, false);
    pal_music_stop();
    pal_music_pause();
    pal_music_resume();

    pal_quit();
}

TEST(sdl_music_is_playing) {
    setup_headless();
    ASSERT(pal_init(PAL_BACKEND_SDL2));

    // Should return false when nothing is playing
    ASSERT(!pal_music_is_playing());

    pal_quit();
}

TEST(sdl_music_volume) {
    setup_headless();
    ASSERT(pal_init(PAL_BACKEND_SDL2));

    // Should not crash
    pal_music_set_volume(0.0f);
    pal_music_set_volume(0.5f);
    pal_music_set_volume(1.0f);

    pal_quit();
}

TEST(sdl_master_volume) {
    setup_headless();
    ASSERT(pal_init(PAL_BACKEND_SDL2));

    // Should not crash
    pal_set_master_volume(0.0f);
    pal_set_master_volume(0.5f);
    pal_set_master_volume(1.0f);

    pal_quit();
}

// =============================================================================
// Font Tests
// =============================================================================

TEST(sdl_font_load_missing) {
    setup_headless();
    ASSERT(pal_init(PAL_BACKEND_SDL2));

    // Loading missing font might return NULL or fallback
    PalFont* font = pal_font_load("nonexistent_font.ttf", 16);
    // Result depends on implementation - just don't crash
    if (font) pal_font_destroy(font);

    pal_quit();
}

TEST(sdl_font_default) {
    setup_headless();
    ASSERT(pal_init(PAL_BACKEND_SDL2));

    // Default font should work (uses system fonts)
    PalFont* font = pal_font_default(16);
    // May be NULL in headless mode with no system fonts
    if (font) {
        pal_font_destroy(font);
    }

    pal_quit();
}

TEST(sdl_font_destroy_null) {
    setup_headless();
    ASSERT(pal_init(PAL_BACKEND_SDL2));

    // Should not crash
    pal_font_destroy(NULL);

    pal_quit();
}

TEST(sdl_draw_text_null) {
    setup_headless();
    ASSERT(pal_init(PAL_BACKEND_SDL2));

    PalWindow* window = pal_window_create("Test", 640, 480);
    ASSERT_NOT_NULL(window);

    // Should not crash with NULL font or window
    pal_draw_text(window, NULL, "Hello", 100, 100, 255, 255, 255, 255);
    pal_draw_text(NULL, NULL, "Hello", 100, 100, 255, 255, 255, 255);

    pal_window_destroy(window);
    pal_quit();
}

TEST(sdl_text_size_null) {
    setup_headless();
    ASSERT(pal_init(PAL_BACKEND_SDL2));

    int width = 999, height = 999;
    pal_text_size(NULL, "Test", &width, &height);
    // With NULL font, uses fallback approximation (8 pixels per char)
    ASSERT(width >= 0);  // Just verify it doesn't crash and returns reasonable value

    pal_quit();
}

// =============================================================================
// Time Tests
// =============================================================================

TEST(sdl_time_monotonic) {
    setup_headless();
    ASSERT(pal_init(PAL_BACKEND_SDL2));

    double t1 = pal_time();
    ASSERT(t1 >= 0.0);

    // Small delay
    for (volatile int i = 0; i < 100000; i++) { }

    double t2 = pal_time();
    ASSERT(t2 >= t1);  // Time should not go backwards

    pal_quit();
}

TEST(sdl_sleep) {
    setup_headless();
    ASSERT(pal_init(PAL_BACKEND_SDL2));

    double t1 = pal_time();
    pal_sleep(0.01);  // 10ms
    double t2 = pal_time();

    // Should have slept at least a little
    ASSERT(t2 > t1);

    pal_quit();
}

// =============================================================================
// Integration Tests
// =============================================================================

TEST(sdl_window_lifecycle) {
    setup_headless();
    ASSERT(pal_init(PAL_BACKEND_SDL2));

    // Create window
    PalWindow* window = pal_window_create("Test", 800, 600);
    ASSERT_NOT_NULL(window);

    // Simulate a few frames
    for (int frame = 0; frame < 5; frame++) {
        pal_poll_events();

        pal_window_clear(window, 30, 30, 30);

        // Draw some shapes
        pal_draw_rect(window, 100 + frame * 10, 100, 50, 50, 255, 0, 0, 255);
        pal_draw_circle(window, 400, 300, 30 + frame * 5, 0, 255, 0, 255);
        pal_draw_line(window, 0, 0, 800, 600, 0, 0, 255, 255);

        pal_window_present(window);
    }

    // Cleanup
    pal_window_destroy(window);
    pal_quit();
}

TEST(sdl_full_render_loop) {
    setup_headless();
    ASSERT(pal_init(PAL_BACKEND_SDL2));

    PalWindow* window = pal_window_create("Game", 640, 480);
    ASSERT_NOT_NULL(window);

    int x = 100, y = 100;

    // Simulate game loop
    for (int i = 0; i < 10 && !pal_should_quit(); i++) {
        pal_poll_events();

        // Check input
        if (pal_key_down(PAL_KEY_LEFT)) x -= 5;
        if (pal_key_down(PAL_KEY_RIGHT)) x += 5;
        if (pal_key_down(PAL_KEY_UP)) y -= 5;
        if (pal_key_down(PAL_KEY_DOWN)) y += 5;

        // Render
        pal_window_clear(window, 0, 0, 0);
        pal_draw_rect(window, x, y, 32, 32, 255, 255, 0, 255);
        pal_window_present(window);
    }

    pal_window_destroy(window);
    pal_quit();
}

// =============================================================================
// Main
// =============================================================================

int main(void) {
    TEST_SUITE("SDL Initialization");
    RUN_TEST(sdl_init_with_dummy_drivers);
    RUN_TEST(sdl_quit_cleanup);
    RUN_TEST(sdl_double_init);

    TEST_SUITE("SDL Window");
    RUN_TEST(sdl_window_create);
    RUN_TEST(sdl_window_destroy_null);
    RUN_TEST(sdl_window_get_size);
    RUN_TEST(sdl_window_get_size_null);
    RUN_TEST(sdl_window_set_title);
    RUN_TEST(sdl_window_clear);
    RUN_TEST(sdl_window_present);

    TEST_SUITE("SDL Drawing");
    RUN_TEST(sdl_draw_rect);
    RUN_TEST(sdl_draw_rect_outline);
    RUN_TEST(sdl_draw_line);
    RUN_TEST(sdl_draw_circle);
    RUN_TEST(sdl_draw_circle_outline);
    RUN_TEST(sdl_draw_edge_cases);

    TEST_SUITE("SDL Textures");
    RUN_TEST(sdl_texture_load_missing);
    RUN_TEST(sdl_texture_destroy_null);
    RUN_TEST(sdl_texture_get_size_null);
    RUN_TEST(sdl_draw_texture_null);
    RUN_TEST(sdl_draw_texture_ex_null);
    RUN_TEST(sdl_draw_texture_region_null);

    TEST_SUITE("SDL Input");
    RUN_TEST(sdl_poll_events);
    RUN_TEST(sdl_should_quit_initial);
    RUN_TEST(sdl_key_down_initial);
    RUN_TEST(sdl_key_pressed_released);
    RUN_TEST(sdl_mouse_position);
    RUN_TEST(sdl_mouse_buttons);
    RUN_TEST(sdl_input_state_between_frames);

    TEST_SUITE("SDL Audio");
    RUN_TEST(sdl_sound_load_missing);
    RUN_TEST(sdl_sound_destroy_null);
    RUN_TEST(sdl_sound_play_null);
    RUN_TEST(sdl_music_load_missing);
    RUN_TEST(sdl_music_destroy_null);
    RUN_TEST(sdl_music_controls_null);
    RUN_TEST(sdl_music_is_playing);
    RUN_TEST(sdl_music_volume);
    RUN_TEST(sdl_master_volume);

    TEST_SUITE("SDL Fonts");
    RUN_TEST(sdl_font_load_missing);
    RUN_TEST(sdl_font_default);
    RUN_TEST(sdl_font_destroy_null);
    RUN_TEST(sdl_draw_text_null);
    RUN_TEST(sdl_text_size_null);

    TEST_SUITE("SDL Time");
    RUN_TEST(sdl_time_monotonic);
    RUN_TEST(sdl_sleep);

    TEST_SUITE("SDL Integration");
    RUN_TEST(sdl_window_lifecycle);
    RUN_TEST(sdl_full_render_loop);

    TEST_SUMMARY();
}

#else // PAL_USE_SDL2

#include <stdio.h>

// Stub for when SDL2 is not available
int main(void) {
    printf("SDL2 not available - skipping SDL tests\n");
    return 0;
}

#endif // PAL_USE_SDL2
