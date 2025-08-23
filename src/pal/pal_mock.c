_play");
}

void pal_mock_sound_play_volume(PalSound* sound, float volume) {
    (void)sound; (void)volume;
    record_call("pal_sound_play_volume");
}

// -----------------------------------------------------------------------------
// Audio - Music
// -----------------------------------------------------------------------------

PalMusic* pal_mock_music_load(const char* path) {
    record_call("pal_music_load");

    PalMusic* music = malloc(sizeof(PalMusic));
    if (!music) return NULL;

    strncpy(music->path, path ? path : "", sizeof(music->path) - 1);
    music->path[sizeof(music->path) - 1] = '\0';

    return music;
}

void pal_mock_music_destroy(PalMusic* music) {
    record_call("pal_music_destroy");
    free(music);
}

void pal_mock_music_play(PalMusic* music, bool loop) {
    (void)loop;
    record_call("pal_music_play");
    mock_current_music = music;
    mock_music_playing = true;
    mock_music_paused = false;
}

void pal_mock_music_stop(void) {
    record_call("pal_music_stop");
    mock_music_playing = false;
    mock_music_paused = false;
}

void pal_mock_music_pause(void) {
    record_call("pal_music_pause");
    if (mock_music_playing) {
        mock_music_paused = true;
    }
}

void pal_mock_music_resume(void) {
    record_call("pal_music_resume");
    mock_music_paused = false;
}

void pal_mock_music_set_volume(float volume) {
    (void)volume;
    record_call("pal_music_set_volume");
    mock_music_volume = volume;
}

bool pal_mock_music_is_playing(void) {
    return mock_music_playing && !mock_music_paused;
}

// -----------------------------------------------------------------------------
// Audio - Master volume
// -----------------------------------------------------------------------------

void pal_mock_set_master_volume(float volume) {
    record_call("pal_set_master_volume");
    mock_master_volume = volume;
}

// -----------------------------------------------------------------------------
// Time
// -----------------------------------------------------------------------------

double pal_mock_time(void) {
    return (double)clock() / CLOCKS_PER_SEC - mock_start_time;
}

void pal_mock_sleep(double seconds) {
    (void)seconds;
    record_call("pal_sleep");
    // No actual sleep in mock - tests should run fast
}

// -----------------------------------------------------------------------------
// Mock input simulation
// -----------------------------------------------------------------------------

void pal_mock_set_key(PalKey key, bool down) {
    if (key >= 0 && key < PAL_KEY_COUNT) {
        mock_keys_down[key] = down;
    }
}

void pal_mock_set_mouse_button(PalMouseButton button, bool down) {
    if (button >= 1 && button <= 3) {
        mock_mouse_down[button] = down;
 

    strncpy(font->path, path ? path : "", sizeof(font->path) - 1);
    font->path[sizeof(font->path) - 1] = '\0';
    font->size = size;
    font->is_default = false;

    return font;
}

PalFont* pal_mock_font_default(int size) {
    record_call("pal_font_default");

    PalFont* font = malloc(sizeof(PalFont));
    if (!font) return NULL;

    font->path[0] = '\0';
    font->size = size;
    font->is_default = true;

    return font;
}

void pal_mock_font_destroy(PalFont* font) {
    record_call("pal_font_destroy");
    free(font);
}

void pal_mock_draw_text(PalWindow* window, PalFont* font, const char* text,
                        int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    (void)window; (void)font; (void)text;
    (void)x; (void)y; (void)r; (void)g; (void)b; (void)a;
    record_call("pal_draw_text");
}

void pal_mock_text_size(PalFont* font, const char* text, int* width, int* height) {
    // Approximate text size: 8 pixels per character width, font size for height
    int len = text ? (int)strlen(text) : 0;
    int char_width = font ? (font->size / 2) : 8;
    int char_height = font ? font->size : 16;

    if (width) *width = len * char_width;
    if (height) *height = char_height;
}
