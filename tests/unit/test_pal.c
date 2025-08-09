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
